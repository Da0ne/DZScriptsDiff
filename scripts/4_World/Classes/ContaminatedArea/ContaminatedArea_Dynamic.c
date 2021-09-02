enum eAreaDecayStage
{
	INIT 			= 1, // The dynamic area is initializing
	START 			= 2, // The dynamic area is starting
	LIVE 			= 3, // The dynamic area is live
	DECAY_START 	= 4, // The dynamic area decay has started
	DECAY_END 		= 5, // The dynamic area will soon be deleted
}

// The parameters for the explosion light when creating dynamic area
class ShellLight extends PointLightBase
{
	protected float m_DefaultBrightness = 10;
	protected float m_DefaultRadius = 100;
	
	void ShellLight()
	{
		SetVisibleDuringDaylight(false);
		SetRadiusTo(m_DefaultRadius);
		SetBrightnessTo(m_DefaultBrightness);
		SetFlareVisible(false);
		SetAmbientColor(1.0, 1.0, 0.3);
		SetDiffuseColor(1.0, 1.0, 0.3);
		SetLifetime(0.15);
		SetDisableShadowsWithinRadius(-1);
		SetCastShadow( false );
	}
}

// The dynamic Contaminated area, using it's own default settings
class ContaminatedArea_Dynamic : EffectArea
{
	protected ref Timer 	m_StartupTimer;
	protected ref Timer 	m_FXTimer;
	protected FlareLight 	m_FlareLight;
	protected ShellLight	m_ShellLight; 				// Light used upon ariborne shell detonation
	protected vector 		m_OffsetPos; 				// This will be the position at which we spawn all future airborne FX
	protected int 			m_DecayState 				= eAreaDecayStage.INIT; // The current state in which the area is
	
	// Constants used for startup events
	const int 				AIRBORNE_EXPLOSION_DELAY 	= 20;
	const int 				AREA_SETUP_DELAY 			= 10;
	const float 			AIRBORNE_FX_OFFSET 			= 50;
	const float 			ARTILLERY_SHELL_SPEED		= 100; // Units per second
	
	// Constants used for dissapearing events
	const float				DECAY_START_PART_SIZE 		= 48;
	const int				DECAY_START_PART_BIRTH_RATE = 1;
	const float				DECAY_END_PART_SIZE 		= 32;
	const int				DECAY_END_PART_BIRTH_RATE 	= 1;
	const float 			START_DECAY_LIFETIME		= 900;
	const float 			FINISH_DECAY_LIFETIME		= 300;
	
	void ContaminatedArea_Dynamic()
	{
		RegisterNetSyncVariableInt("m_DecayState");
	}
	
	override void EEOnCECreate()
	{
		// We get the PPE index for future usage and synchronization ( we must do it here for dynamic as it is not read through file )
		if ( GetGame().IsServer() )
			m_PPERequesterIdx = PPERequesterBank.GetRequester( m_PPERequesterType.ToType() ).GetRequesterIDX();
		
		SetSynchDirty();
		
		// If this is the first initialization, we delay it in order to play cool effects
		if ( m_DecayState == eAreaDecayStage.INIT )
		{
			vector areaPos = GetPosition();
			m_OffsetPos = areaPos;
			m_OffsetPos[1] = m_OffsetPos[1] + AIRBORNE_FX_OFFSET;
			
			// play artillery sound, sent to be played for everyone on server
			array<vector> artilleryPoints = GetGame().GetMission().GetWorldData().GetArtyFiringPos();
			vector closestPoint = areaPos;
			int dist = 0;
			int temp;
			int index = 0;
			for ( int i = 0; i < artilleryPoints.Count(); i++ )
			{
				temp = vector.DistanceSq( artilleryPoints.Get( i ), areaPos );
				if ( temp < dist || dist == 0 )
				{
					dist = temp;
					index = i;
				}
			}
			
			closestPoint = artilleryPoints.Get( index );
			
			// We calculate the delay depending on distance from firing position to simulate shell travel time
			float delay = vector.Distance( closestPoint, areaPos );
			delay = delay / ARTILLERY_SHELL_SPEED;
			delay += AIRBORNE_EXPLOSION_DELAY; // We add the base, minimum time ( no area can start before this delay )
			
			Param3<vector, vector, float> pos; // The value to be sent through RPC
			array<ref Param> params; // The RPC params
			
			// We prepare to send the message
			pos = new Param3<vector, vector, float>( closestPoint, areaPos, delay );
			params = new array<ref Param>;
			
			// We send the message with this set of coords
			params.Insert( pos );
			GetGame().RPC( null, ERPCs.RPC_SOUND_ARTILLERY_SINGLE, params, true );
			
			m_FXTimer = new Timer( CALL_CATEGORY_GAMEPLAY );
			m_FXTimer.Run( delay, this, "PlayFX" );	
			
			delay += AREA_SETUP_DELAY; // We have an additional delay between shell detonation and finalization of area creation
			// setup zone
			m_StartupTimer = new Timer( CALL_CATEGORY_GAMEPLAY );
			m_StartupTimer.Run( delay, this, "InitZone" );
		}
	}
	
	// Through this we will evaluate the resize of particles
	override void OnCEUpdate()
	{
		super.OnCEUpdate();

		// The first state of decay, slight reduction in particle density and size
		if ( GetLifetime() < START_DECAY_LIFETIME )
		{
			SetDecayState( eAreaDecayStage.DECAY_START );
		}
		
		// The second state of decay, further reduction of particle density and size
		if ( GetLifetime() < FINISH_DECAY_LIFETIME )
		{
			SetDecayState( eAreaDecayStage.DECAY_END );
		}
		
		SetSynchDirty();
	}
	
	// Set the new state of the Area
	void SetDecayState( int newState )
	{
		m_DecayState = newState;
		
		// We update the trigger state values as we also want to update player bound effects
		if ( m_Trigger )
			ContaminatedTrigger_Dynamic.Cast( m_Trigger ).SetAreaState( m_DecayState );
	}
	
	override void EEInit()
	{
		// We make sure we have the particle array
		if ( !m_ToxicClouds )
			m_ToxicClouds = new array<Particle>;
		
		// We set the values for dynamic area as these are not set through JSON and are standardized
		m_Name = "Default Dynamic";
		m_Radius = 120;
		m_PositiveHeight = 7;
		m_NegativeHeight = 10;
		m_InnerRings = 1;
		m_InnerSpacing = 40;
		m_OuterSpacing = 30;
		m_OuterRingOffset = 0;
		m_Type = eZoneType.DYNAMIC;
		m_TriggerType = "ContaminatedTrigger_Dynamic";
		
		SetSynchDirty();
		
		#ifdef DEVELOPER
		// Debugs when placing entity by hand using internal tools
		/*if ( GetGame().IsServer() && !GetGame().IsMultiplayer() )
		{
			Debug.Log("YOU CAN IGNORE THE FOLLOWING DUMP");
			InitZone();
			Debug.Log("YOU CAN USE FOLLOWING DATA PROPERLY");
		}*/
		#endif
		
		m_OffsetPos = GetPosition();
		m_OffsetPos[1] = m_OffsetPos[1] + AIRBORNE_FX_OFFSET;
		
		// If a player arrives slightly later during the creation process we check if playing the flare FX is relevant
		if ( m_DecayState == eAreaDecayStage.INIT )
			PlayFlareVFX();
		
		if ( m_DecayState == eAreaDecayStage.LIVE )
			InitZone(); // If it has already been created, we simply do the normal setup, no cool effects, force the LIVE state
		else if ( GetGame().IsClient() && m_DecayState > eAreaDecayStage.LIVE )
			InitZoneClient(); // Same as before but without state forcing
		
		super.EEInit();
	}
	
	// We spawn particles and setup trigger
	override void InitZone()
	{
		m_DecayState = eAreaDecayStage.LIVE;
		SetSynchDirty();
		
		super.InitZone();
	}
	
	override void InitZoneServer()
	{
		super.InitZoneServer();
		
		// We create the trigger on server
		if ( m_TriggerType != "" )
			CreateTrigger( m_Position, m_Radius );
	}
	
	override void InitZoneClient()
	{
		super.InitZoneClient();
		
		if ( !m_ToxicClouds )
			m_ToxicClouds = new array<Particle>;
		
		// We spawn VFX on client
		PlaceParticles( GetWorldPosition(), m_Radius, m_InnerRings, m_InnerSpacing, m_OuterRingToggle, m_OuterSpacing, m_OuterRingOffset, m_ParticleID );
	
		if ( m_DecayState > eAreaDecayStage.LIVE )
		{
			for ( int i = 0; i < m_ToxicClouds.Count(); i++ )
			{
				if ( m_DecayState == eAreaDecayStage.DECAY_END )
				{
					m_ToxicClouds.Get( i ).SetParameter( 0, EmitorParam.BIRTH_RATE, DECAY_END_PART_BIRTH_RATE );
					m_ToxicClouds.Get( i ).SetParameter( 0, EmitorParam.SIZE, DECAY_END_PART_SIZE );
				}
				else
				{
					m_ToxicClouds.Get( i ).SetParameter( 0, EmitorParam.BIRTH_RATE, DECAY_START_PART_BIRTH_RATE );
					m_ToxicClouds.Get( i ).SetParameter( 0, EmitorParam.SIZE, DECAY_START_PART_SIZE );
				}
			}
		}
	}
	
	override void CreateTrigger( vector pos, int radius )
	{
		super.CreateTrigger( pos, radius );
		
		// This handles the specific case of dynamic triggers as some additionnal parameters are present
		ContaminatedTrigger_Dynamic dynaTrigger = ContaminatedTrigger_Dynamic.Cast( m_Trigger );
		if ( dynaTrigger )
		{
			dynaTrigger.SetLocalEffects( m_AroundParticleID, m_TinyParticleID, m_PPERequesterIdx );
			dynaTrigger.SetAreaState( m_DecayState );
		}
	}
	
	void PlayFX()
	{
		if ( GetGame().IsServer() )
		{
			Param1<vector> pos; // The value to be sent through RPC
			array<ref Param> params; // The RPC params
			
			// We prepare to send the message
			pos = new Param1<vector>( vector.Zero );
			params = new array<ref Param>;
			
			// We send the message with this set of coords
			pos.param1 = m_OffsetPos;
			params.Insert( pos );
			GetGame().RPC( null, ERPCs.RPC_SOUND_CONTAMINATION, params, true );
			
			// We go to the next stage
			m_DecayState = eAreaDecayStage.START;
			SetSynchDirty();
		}
	}
	
	void PlayExplosionLight()
	{
		m_ShellLight = ShellLight.Cast( ScriptedLightBase.CreateLight( ShellLight, m_OffsetPos ) );
	}
	
	void PlayFlareVFX()
	{
		if ( GetGame().IsClient() || ( GetGame().IsServer() && !GetGame().IsMultiplayer() ) )
		{
			// We spawn locally the dummy object which will be used to move and manage the particle
			DynamicArea_Flare dummy = DynamicArea_Flare.Cast( GetGame().CreateObjectEx( "DynamicArea_Flare", m_OffsetPos, ECE_SETUP | ECE_LOCAL ) );
			
			// We add some light to reinforce the effect
			m_FlareLight = FlareLightContamination.Cast(ScriptedLightBase.CreateLight( FlareLightContamination, m_OffsetPos ));
		}
	}
	
	override void EEDelete( EntityAI parent )
	{
		super.EEDelete( parent );
	}
	
	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		
		if ( !m_ToxicClouds )
			return;
		
		switch ( m_DecayState )
		{
			case eAreaDecayStage.START:
				PlayExplosionLight();
			break;
			case eAreaDecayStage.LIVE:
				InitZoneClient();
			
			break;
			case eAreaDecayStage.DECAY_START:
			{
				// We go through all the particles bound to this area and update relevant parameters
				//Debug.Log("We start decay");
				for ( int i = 0; i < m_ToxicClouds.Count(); i++ )
				{
					m_ToxicClouds.Get( i ).SetParameter( 0, EmitorParam.BIRTH_RATE, DECAY_START_PART_BIRTH_RATE );
					m_ToxicClouds.Get( i ).SetParameter( 0, EmitorParam.SIZE, DECAY_START_PART_SIZE );
				}
				
				break;
			}
			case eAreaDecayStage.DECAY_END:
			{
				// We go through all the particles bound to this area and update relevant parameters
				//Debug.Log("We finish decay");
				for ( int j = 0; j < m_ToxicClouds.Count(); j++ )
				{
					m_ToxicClouds.Get( j ).SetParameter( 0, EmitorParam.BIRTH_RATE, DECAY_END_PART_BIRTH_RATE );
					m_ToxicClouds.Get( j ).SetParameter( 0, EmitorParam.SIZE, DECAY_END_PART_SIZE );
				}
				
				break;
			}
			default:
			break;
		}
	}
}