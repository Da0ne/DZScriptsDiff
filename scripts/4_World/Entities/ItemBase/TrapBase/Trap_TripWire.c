// Wire type is used in the case of decrafting to give back the correct base Ingredient
enum eWireMaterial
{
	WIRE 		= 0,
	BARBED_WIRE = 1,
	ROPE 		= 2
}

class TripwireTrap : TrapBase
{
	int 		m_State = FOLDED;
	private int m_WireMaterial;
	
	// Current state of the tripwire
	static const int 	FOLDED = 3;
	static const int 	DEPLOYED = 2;
	static const int 	TRIGGERED = 1;
	
	void TripwireTrap()
	{
		m_DamagePlayers = 0; 			//How much damage player gets when caught
		m_InitWaitTime = 0; 			//After this time after deployment, the trap is activated
		m_DefectRate = 15;
		m_NeedActivation = false;
		m_AnimationPhaseGrounded = "inventory";
		m_AnimationPhaseSet = "placing";
		m_AnimationPhaseTriggered = "triggered";
		m_InfoActivationTime = string.Format("#STR_TripwireTrap0%1#STR_TripwireTrap1", m_InitWaitTime.ToString());  // nefunguje dynamicke vyrazy mimo funkcii	
	
		RegisterNetSyncVariableInt("m_State");
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
				
		if ( IsPlaceSound() )
		{
			PlayPlaceSound();
		}
	}
	
	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);
		
		ctx.Write( m_State );
	}
	
	//----------------------------------------------------------------
	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{   
		if ( !super.OnStoreLoad(ctx, version) )
			return false;
		
		int state = FOLDED;
		if ( !ctx.Read( state ) )
			state = FOLDED;
		
		SetState( state );
		RefreshState();

		return true;
	}
	
	override void CreateTrigger()
	{
		m_TrapTrigger = TripWireTrigger.Cast( GetGame().CreateObject( "TripWireTrigger", GetPosition(), false ) );
		vector mins = "-0.75 0.3 -0.01";
		vector maxs = "0.75 0.32 0.01";
		m_TrapTrigger.SetOrientation( GetOrientation() );
		m_TrapTrigger.SetExtents(mins, maxs);	
		m_TrapTrigger.SetParentObject( this );
	}
	
	override void OnSteppedOn(EntityAI victim)
	{
		SetState(TRIGGERED);
		
		// We must deal some damage, here 5 shock as melee damage in order to trigger hit animation
		if ( GetGame().IsServer() && victim )
			victim.ProcessDirectDamage(DT_CLOSE_COMBAT, this, "", "TripWireHit", "0 0 0", 1);
		
		// We play the trap trigger sound
		if ( GetGame().IsClient() || !GetGame().IsMultiplayer() )
		{
			EffectSound sound = SEffectManager.PlaySound("TripwireTrap_Trigger_SoundSet", GetPosition(), 0, 0, false);
			sound.SetSoundAutodestroy( true );
		}
	}
	
	override void OnItemLocationChanged( EntityAI old_owner, EntityAI new_owner ) 
	{
		super.OnItemLocationChanged( old_owner, new_owner );
		
		PlayerBase player = PlayerBase.Cast( new_owner );
		if ( player )
			StartDeactivate( player );
	}
	
	void SetState(int state_ID)
	{
		m_State = state_ID;
	}
	
	int GetState()
	{
		return m_State;
	}
	
	void SetWireType( int wireType )
	{
		m_WireMaterial = wireType;
	}
	
	int GetWireType()
	{
		return m_WireMaterial;
	}
	
	override void RefreshState()
	{
		super.RefreshState();
		
		if ( GetState() == FOLDED )
		{
			FoldTripWire();
		}
	}
	
	override void SetupTrapPlayer( PlayerBase player, bool set_position = true )
	{
		super.SetupTrapPlayer( player, set_position );
		SetState(DEPLOYED);
	}
	
	override void StartDeactivate( PlayerBase player )
	{
		super.StartDeactivate(player);
		SetState(FOLDED);
	}
	
	// We do not want players to attach charges before trap is deployed
	override bool CanReceiveAttachment( EntityAI attachment, int slotId )
	{
		if ( GetState() != DEPLOYED )
			return false;
		return super.CanReceiveAttachment( attachment, slotId );
	}
	
	// As players cannot attch charges, we do not display the attachment slot before it is necessary
	override bool CanDisplayAttachmentSlot( string slot_name )
	{
		if ( GetState() != DEPLOYED )
			return false;
		return super.CanDisplayAttachmentSlot( slot_name );
	}
	
	override void EEItemAttached(EntityAI item, string slot_name)
	{
		super.EEItemAttached(item, slot_name);
	}
	
	override void EEItemDetached(EntityAI item, string slot_name)
	{
		super.EEItemDetached(item, slot_name);
	}
	
	override void EEKilled( Object killer )
	{
		if ( m_TrapTrigger )
			StartDeactivate( null );
	}
	
	// We reset the animation phases to see the tripwire as folded
	void FoldTripWire()
	{
		if ( m_AnimationPhaseGrounded != "" )
		{
			SetAnimationPhase( m_AnimationPhaseSet, 1 );
			
			if ( m_AnimationPhaseTriggered != m_AnimationPhaseGrounded ) 
			{
				SetAnimationPhase( m_AnimationPhaseTriggered, 1 );
			}
			
			SetAnimationPhase( m_AnimationPhaseGrounded, 0 );
		}
	}
	
	override void OnInventoryEnter( Man player )
	{
		SetState( FOLDED );
	}
	
	// How one sees the tripwire when in vicinity
	override int GetViewIndex()
	{
		if ( MemoryPointExists( "invView2" ) )
		{
			#ifdef PLATFORM_WINDOWS
			InventoryLocation il = new InventoryLocation;
			GetInventory().GetCurrentInventoryLocation( il );
			InventoryLocationType type = il.GetType();
			switch ( type )
			{
				case InventoryLocationType.CARGO:
				{
					return 0;
				}
				case InventoryLocationType.ATTACHMENT:
				{
					return 1;
				}
				case InventoryLocationType.HANDS:
				{
					return 0;
				}
				case InventoryLocationType.GROUND:
				{
					// Different view index depending on deployment state 
					if ( GetState() == DEPLOYED )
						return 1;
					else if ( GetState() == TRIGGERED )
						return 2;
					
					// When folded
					return 0;
				}
				case InventoryLocationType.PROXYCARGO:
				{
					return 0;
				}
				default:
				{
					if ( GetState() == DEPLOYED )
						return 1;
					else if ( GetState() == TRIGGERED )
						return 2;
					
					// When folded
					return 0;
				}
			}
			#ifdef PLATFORM_CONSOLE
			return 1;
			#endif
			#endif
		}
		return 0;
	}
	
	//================================================================
	// ADVANCED PLACEMENT
	//================================================================
	
	// On placement complete, set state, play sound, create trigger and synch to client
	override void OnPlacementComplete( Man player, vector position = "0 0 0", vector orientation = "0 0 0" )
	{
		super.OnPlacementComplete( player, position, orientation );
		
		SetIsPlaceSound( true );
		if ( GetGame().IsServer() )
		{
			SetState( DEPLOYED );
			PlayerBase player_PB = PlayerBase.Cast( player );
			StartActivate( player_PB );
			
			m_TrapTrigger.SetPosition( position );
			m_TrapTrigger.SetOrientation( orientation );
			
			SetSynchDirty();
		}
	}
	
	override void OnPlacementCancelled( Man player )
	{
		super.OnPlacementCancelled( player );
		
		SetState( FOLDED );
	}
	
	override bool IsDeployable()
	{
		return true;
	}
	
	// Tripwire cannot be taken if deployed with attachment
	override bool IsTakeable()
	{
		if ( GetState() != DEPLOYED || ( GetInventory().AttachmentCount() == 0 && GetState() == DEPLOYED ) )
		{
			return true;
		}
		
		return false;
	}
	
	override string GetDeploySoundset()
	{
		return "tripwire_deploy_SoundSet";
	}
	
	override string GetLoopDeploySoundset()
	{
		return "tripwiretrap_deploy_SoundSet";
	}
	
	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionTogglePlaceObject);
		AddAction(ActionDeployObject);
	}
	
	// ====================================
	// ===========  DEPRECATED  ===========
	// ====================================
	
	void UpdateProxySelections()
	{
		if ( GetInventory().AttachmentCount() > 0)
		{
			ItemBase attachment = ItemBase.Cast( GetInventory().GetAttachmentFromIndex(0) );
			
			if ( attachment )
			{
				// Hide all proxies
				for (int i = 1; i <= 3; i++)
				{
					HideSelection("s" + i + "_charge");
				}
				
				// Now show the one we need to see
				string proxy_to_show = string.Format("s%1_charge", GetState() ); 
				//Print(proxy_to_show);
				ShowSelection( proxy_to_show );
			}
		}
	}
}

class TripwireTrapDeployed : TripwireTrap
{
	
}
