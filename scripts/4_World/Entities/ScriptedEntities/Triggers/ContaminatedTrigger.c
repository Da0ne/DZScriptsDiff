// In this file you will find both Static and Dynamic contaminated area triggers
class ContaminatedTrigger : EffectTrigger
{
	const float DAMAGE_TICK_RATE = 10;//deal damage every n in seconds
	void ContaminatedTrigger()
	{
		// Register this in the trigger effect manager
		m_Manager = TriggerEffectManager.GetInstance();
		m_Manager.SetTriggerType( this );
	}
	
	// ----------------------------------------------
	// 				TRIGGER EVENTS
	// ----------------------------------------------
	
	override void OnEnterServerEvent( TriggerInsider insider )
	{
		super.OnEnterServerEvent( insider );
		
		// We don't need to test the trigger count as Modifiers handle such cases already
		if ( insider.GetObject().IsInherited( PlayerBase ) )
		{
			PlayerBase playerInsider = PlayerBase.Cast( insider.GetObject() );
			playerInsider.GetModifiersManager().ActivateModifier( eModifiers.MDF_AREAEXPOSURE );
			
			m_Manager.OnPlayerEnter( playerInsider, this );
		}
		
		
		DayZCreatureAI creature = DayZCreatureAI.Cast( insider.GetObject() );
		
		if(creature)
			creature.IncreaseEffectTriggerCount();

		
	}
	
	override void OnEnterClientEvent( TriggerInsider insider )
	{
		if ( insider.GetObject().IsInherited( PlayerBase ) )
		{
			PlayerBase playerInsider = PlayerBase.Cast( insider.GetObject() );
			
			// We will only handle the controlled player, as effects are only relevant to this player instance
			if ( playerInsider.IsControlledPlayer() )
			{
				// If it is first entrance, we fire the following
				if ( !m_Manager.IsPlayerInTriggerType( playerInsider, this ) )
					playerInsider.SetContaminatedEffect( true, m_PPERequester, m_AroundPartId, m_TinyPartId );
				
				// We then handle the update of player trigger state in manager
				m_Manager.OnPlayerEnter( playerInsider, this );
			}
		}
		
		super.OnEnterClientEvent( insider );
	}
	
	override void OnLeaveServerEvent( TriggerInsider insider )
	{
		if ( insider && insider.GetObject().IsInherited( PlayerBase ) )
		{
			PlayerBase playerInsider = PlayerBase.Cast( insider.GetObject() );
			
			// We first handle the update of player trigger state in manager
			m_Manager.OnPlayerExit( playerInsider, this );
			// We test if player is still in designated trigger type
			if ( !m_Manager.IsPlayerInTriggerType( playerInsider, this ) )
				playerInsider.GetModifiersManager().DeactivateModifier( eModifiers.MDF_AREAEXPOSURE );
		}
		
		DayZCreatureAI creature = DayZCreatureAI.Cast( insider.GetObject() );
		
		if(creature)
			creature.DecreaseEffectTriggerCount();
		
		super.OnLeaveServerEvent( insider );
	}
	
	override void OnLeaveClientEvent( TriggerInsider insider )
	{
		if ( insider.GetObject().IsInherited( PlayerBase ) )
		{
			// Make sure you pass the set variable for PPE effect
			// It will not remove the correct one if START and STOP don't point to the same Requester
			PlayerBase playerInsider = PlayerBase.Cast( insider.GetObject() );
			
			// We will only handle the controlled player, as effects are only relevant to this player instance
			if ( playerInsider.IsControlledPlayer() )
			{
				// We first handle the update of player trigger state in manager
				m_Manager.OnPlayerExit( playerInsider, this );
				
				// We test if player is still in designated trigger type
				if ( !m_Manager.IsPlayerInTriggerType( playerInsider, this ) )
					playerInsider.SetContaminatedEffect( false, m_PPERequester );
			}
		}
		
		super.OnLeaveClientEvent( insider );
	}
	
	override void EOnFrame(IEntity other, float timeSlice)
	{
		super.EOnFrame(other, timeSlice);
		m_DeltaTime = timeSlice;
	}
	
	override void OnStayFinishServerEvent()
	{
		m_TimeAccuStay += m_DeltaTime;
		array<ref TriggerInsider> insiders = GetInsiders();
		if (m_TimeAccuStay > DAMAGE_TICK_RATE && insiders && insiders.Count() > 0)
		{
			foreach( TriggerInsider insider:insiders)
			{
				DayZCreatureAI creature = DayZCreatureAI.Cast( insider.GetObject());
				if(creature && creature.m_EffectTriggerCount != 0)
					creature.DecreaseHealth("", "", GameConstants.AI_CONTAMINATION_DMG_PER_SEC * m_TimeAccuStay / creature.m_EffectTriggerCount);// we devide by m_EffectTriggerCount for multiple trigger presence(overlapping triggers)
			}

			m_TimeAccuStay = 0;
		}
	}
	
	
	override bool CanAddObjectAsInsider(Object object)
	{
		DayZCreatureAI creature = DayZCreatureAI.Cast( object );
		if(creature)
		{
			return !creature.ResistContaminatedEffect();
		}
		else
		{
			PlayerBase player = PlayerBase.Cast(object);
			return player != null;
		}
	}
	
	
}

class ContaminatedTrigger_Dynamic : ContaminatedTrigger
{
	protected int m_AreaState; // Used to get the state from linked area and update effects
	
	void ContaminatedTrigger_Dynamic()
	{
		RegisterNetSyncVariableInt("m_AreaState");
		
		// Register in the trigger effect manager
		m_Manager = TriggerEffectManager.GetInstance();
		m_Manager.SetTriggerType( this );
	}
	
	void SetAreaState( int state )
	{
		m_AreaState = state;
		SetSynchDirty();
	}
	
	override void OnEnterClientEvent( TriggerInsider insider )
	{
		if ( insider.GetObject().IsInherited( PlayerBase ) )
		{
			PlayerBase playerInsider = PlayerBase.Cast( insider.GetObject() );
			
			// We will only handle the controlled player, as effects are only relevant to this player instance
			if ( playerInsider.IsControlledPlayer() )
			{
				// If it is first entrance, we fire the following
				if ( !m_Manager.IsPlayerInTriggerType( playerInsider, this ) )
				{
					// We check if we are not in the standard LIVE state
					bool nonDefaultState = m_AreaState > eAreaDecayStage.LIVE;
					if ( nonDefaultState )
					{
						// if not we check the specific state and update local effect values
						int localPartBirthRate;
						if ( m_AreaState == eAreaDecayStage.DECAY_START )
							localPartBirthRate = 20;
						else
							localPartBirthRate = 10;
						
						playerInsider.SetContaminatedEffect( true, m_PPERequester, m_AroundPartId, m_TinyPartId, nonDefaultState, localPartBirthRate );
					}
					else
						playerInsider.SetContaminatedEffect( true, m_PPERequester, m_AroundPartId, m_TinyPartId );
				}
				
				// We then handle the update of player trigger state in manager
				m_Manager.OnPlayerEnter( playerInsider, this );
			}
		}
	}
	
	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		
		// Is the controlled player inside when trigger is synchronized
		for ( int i = 0; i < GetInsiders().Count(); i++ )
		{
			PlayerBase playerInsider = PlayerBase.Cast( m_insiders.Get( i ).GetObject() );
			if ( playerInsider )
			{
				if ( playerInsider.IsControlledPlayer() )
				{
					// Update Local particle effects
					bool nonDefaultState = m_AreaState > eAreaDecayStage.LIVE;
					if ( nonDefaultState )
					{
						int localPartBirthRate;
						if ( m_AreaState == eAreaDecayStage.DECAY_START )
							localPartBirthRate = 20;
						else
							localPartBirthRate = 10;
						
						// Update the local effects
						playerInsider.SetContaminatedEffect( true, m_PPERequester, m_AroundPartId, m_TinyPartId, nonDefaultState, localPartBirthRate );
					}
				}
			}
		}
	}
	
}