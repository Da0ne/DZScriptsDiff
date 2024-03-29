class Environment
{
	const float RAIN_LIMIT_LOW		= 0.05;

	const float WATER_LEVEL_HIGH	= 1.5;
	const float WATER_LEVEL_MID 	= 1.2;
	const float WATER_LEVEL_LOW 	= 0.5;
	const float WATER_LEVEL_NONE	= 0.15;
	
	protected float					m_WetDryTick; //ticks passed since last clothing wetting or drying
	protected float					m_ItemsWetnessMax; //! keeps wetness of most wet item in player's possesion
	protected float					m_RoofCheckTimer; // keeps info about tick time

	//player
	protected PlayerBase 			m_Player;
	protected float					m_PlayerHeightPos; // y position of player above water level (meters)
	protected float 				m_PlayerSpeed; // 1-3 speed of player movement
	protected float 				m_PlayerTemperature; //34-44
	protected float 				m_PlayerHeat; //3-9 heatcomfort generated by entites movement
	protected float					m_HeatComfort;  //delta that changes entitys temperature				

	//environment
	protected float 				m_Rain = 0; // 0-1 amount of rain 
	protected float 				m_Wind = 0; // strength of wind
	protected float 				m_Fog = 0; // 0-1 how foggy it is
	protected float 				m_DayOrNight = 0; // 0-1 day(1) or night(0)
	protected float 				m_Clouds = 0; // 0-1 how cloudy it is
	protected float 				m_EnvironmentTemperature = 0; //temperature of environment player is in
	protected float					m_Time = 0;
	protected string 				m_SurfaceType;
	
	//
	protected float					m_WaterLevel;
	protected bool 					m_IsUnderRoof;
	private bool					m_IsUnderRoofBuilding;
	protected bool 					m_IsInWater;
	protected bool					m_IsTempSet;
	//
	protected float 				m_HeatSourceTemp;	//! DEPRECATED
	protected float 				m_HeatBufferTimer;
	
	protected ref array<int> 		m_SlotIdsComplete;
	protected ref array<int> 		m_SlotIdsUpper;
	protected ref array<int> 		m_SlotIdsBottom;
	protected ref array<int> 		m_SlotIdsLower;
	
	protected ref array<int>		m_HeadParts;
	protected ref array<int>		m_BodyParts;
	protected ref array<int>		m_FeetParts;
	
	protected ref SimpleMovingAverage<float> m_WindAverageBuffer;

	protected bool m_HasTemperatureSources;
	protected float m_UTSAverageTemperature;
	protected ref array<UTemperatureSource> m_UTemperatureSources;
	protected ref SimpleMovingAverage<float> m_UTSAverageTemperatureBuffer;
	
	#ifdef DIAG_DEVELOPER
	bool m_Debug = false;

	bool m_DebugLogDryWet = false;
	#endif
		
	void Environment(PlayerBase pPlayer)
	{
		Init(pPlayer);
	}
	
	void Init(PlayerBase pPlayer)
	{
		m_Player 						= pPlayer;
		m_PlayerSpeed					= 0.0;
		m_WetDryTick					= 0.0;
		m_RoofCheckTimer				= 0.0;
		m_WaterLevel					= 0.0;
		m_HeatComfort					= 0.0;
		
		m_IsUnderRoof					= false;
		m_IsInWater						= false;
		m_SurfaceType					= "cp_dirt";

		m_HeatBufferTimer 				= 0.0;
		
		m_WindAverageBuffer 			= new SimpleMovingAverage<float>(30, 0.5);
		
		m_UTSAverageTemperature			= 0.0;
		m_UTemperatureSources 			= new array<UTemperatureSource>();
		m_UTSAverageTemperatureBuffer	= new SimpleMovingAverage<float>(10, 0);

		//! whole body slots		
		m_SlotIdsComplete 		= new array<int>;		
		m_SlotIdsComplete 		= {
			InventorySlots.HEADGEAR,
			InventorySlots.MASK,
			InventorySlots.EYEWEAR,
			InventorySlots.GLOVES,
			InventorySlots.ARMBAND,
			InventorySlots.BODY,
			InventorySlots.HIPS,
			InventorySlots.VEST,
			InventorySlots.BACK,
			InventorySlots.LEGS,
			InventorySlots.FEET
		};
		//! upper body part slots
		m_SlotIdsUpper 			= new array<int>;
		m_SlotIdsUpper 			= {
			InventorySlots.GLOVES,
			InventorySlots.ARMBAND,
			InventorySlots.BODY,
			InventorySlots.HIPS,
			InventorySlots.VEST,
			InventorySlots.BACK,
			InventorySlots.LEGS,
			InventorySlots.FEET
		};
		//! bottom body part slots
		m_SlotIdsBottom 		= new array<int>;
		m_SlotIdsBottom			= {
			InventorySlots.HIPS,
			InventorySlots.LEGS,
			InventorySlots.FEET
		};
		//! lower body part slots
		m_SlotIdsLower 			= new array<int>;
		m_SlotIdsLower			= {
			InventorySlots.FEET,
		};

		//! --------------------------
		//! heat comfort related slots
		m_HeadParts				= new array<int>;
		m_HeadParts				= {
			InventorySlots.HEADGEAR,
			InventorySlots.MASK,
		};
		
		m_BodyParts				= new array<int>;
		m_BodyParts				= {
			InventorySlots.GLOVES,

			InventorySlots.BODY,
			InventorySlots.BACK,
			InventorySlots.VEST,
		};
		
		m_FeetParts				= new array<int>;
		m_FeetParts				= {
			InventorySlots.LEGS,
			InventorySlots.FEET,
		};
	}

	
	// Calculates heatisolation of clothing, process its wetness, collects heat from heated items and calculates player's heat comfort
	void Update(float pDelta)
	{
		if (m_Player)
		{
			m_RoofCheckTimer += pDelta;
			//! check if player is under roof (only if the Building check is false)
			if ( m_RoofCheckTimer >= GameConstants.ENVIRO_TICK_ROOF_RC_CHECK )
			{
				if ( !IsInsideBuilding() )
					CheckUnderRoof();
				
				m_RoofCheckTimer = 0;
			}

			m_Time += pDelta;
			if ( m_Time >= GameConstants.ENVIRO_TICK_RATE )
			{
				m_Time = 0;
				m_WetDryTick++; // Sets whether it is time to add wetness to items and clothing

				//! Updates data
				CheckWaterContact(m_WaterLevel);
				CollectAndSetPlayerData();
				CollectAndSetEnvironmentData();
				GatherTemperatureSources();

				ProcessTemperatureSources();
				
				//! Process temperature
				ProcessItemsHeat();

				//! Process item wetness/dryness
				if ( m_WetDryTick >= GameConstants.ENVIRO_TICKS_TO_WETNESS_CALCULATION )
				{
					if ( IsWaterContact() )
					{
						ProcessWetnessByWaterLevel(m_WaterLevel);
					}
					else if ( IsRaining() && !IsInsideBuilding() && !IsUnderRoof() && !IsInsideVehicle() )
					{
						ProcessWetnessByRain();
					}
					else
					{
						ProcessItemsDryness();
					}

					//! setting of wetness/dryiness of player
					if ( ( m_ItemsWetnessMax < GameConstants.STATE_WET ) && ( m_Player.GetStatWet().Get() == 1 ) )
					{
						m_Player.GetStatWet().Set( 0 );
					}
					else if ( ( m_ItemsWetnessMax >= GameConstants.STATE_WET ) && ( m_Player.GetStatWet().Get() == 0 ) )
					{
						m_Player.GetStatWet().Set( 1 );
					}

					m_WetDryTick = 0;
					m_ItemsWetnessMax = 0; //! reset item wetness counter;
				}
			}
		}
	}
	
	bool IsTemperatureSet()
	{
		return m_IsTempSet;
	}

	// DEPRECATED
	void AddToEnvironmentTemperature(float pTemperature);
	
	//! Returns heat player generated based on player's movement speed (for now)
	protected float GetPlayerHeat()
	{
		float heat = Math.Max(m_PlayerSpeed * GameConstants.ENVIRO_DEFAULT_ENTITY_HEAT, GameConstants.ENVIRO_DEFAULT_ENTITY_HEAT);
		return heat;
	}
	
	bool IsUnderRoof()
	{
		return m_IsUnderRoof;
	}
	
	protected bool IsWaterContact()
	{
		return m_IsInWater;
	}
	
	bool IsInsideBuilding()
	{
		return m_Player && m_Player.IsSoundInsideBuilding();
	}

	protected bool IsInsideVehicle()
	{
		return m_Player && m_Player.IsInVehicle();
	}
	
	private bool IsUnderRoofBuilding()
	{
		return m_IsUnderRoofBuilding;
	}
	
	protected bool IsRaining()
	{
		return m_Rain > RAIN_LIMIT_LOW;
	}
	
	//! Checks whether Player is sheltered
	protected void CheckUnderRoof()
	{
		// if inside vehicle return immediatelly
		if (IsInsideVehicle())
		{
			m_IsUnderRoof = false;
			m_IsUnderRoofBuilding = false;
			return;
		}
		
		float hitFraction;
		vector hitPosition, hitNormal;
		vector from = m_Player.GetPosition();
		vector to = from + "0 25 0";
		Object hitObject;
		PhxInteractionLayers collisionLayerMask = PhxInteractionLayers.ITEM_LARGE|PhxInteractionLayers.BUILDING|PhxInteractionLayers.VEHICLE;
		
		m_IsUnderRoof = DayZPhysics.RayCastBullet(from, to, collisionLayerMask, null, hitObject, hitPosition, hitNormal, hitFraction);

		m_IsUnderRoofBuilding = hitObject && hitObject.IsInherited(House);
	}
	
	protected void CheckWaterContact(out float pWaterLevel)
	{
		m_IsInWater = false;
		if (m_Player.IsSwimming())
		{
			m_IsInWater = true;
			HumanMovementState hms = new HumanMovementState();
			m_Player.GetMovementState(hms);
			pWaterLevel = WATER_LEVEL_MID;
			if (hms.m_iMovement >= DayZPlayerConstants.MOVEMENTIDX_WALK)
				pWaterLevel = WATER_LEVEL_HIGH;

			return;
		}
		
		//! no valid surface under character
		if (IsUnderRoofBuilding())
		{
			m_IsInWater = false;
			return;
		}

		string surfType;
		int liquidType;

		g_Game.SurfaceUnderObject(m_Player, surfType, liquidType);
		
		switch ( liquidType )
		{
			case 0: // sea
			case LIQUID_WATER:
			case LIQUID_RIVERWATER:
				pWaterLevel = m_Player.GetCurrentWaterLevel();
				m_IsInWater = true;
			break;
		}

		//! sync info about water contact to player
		m_Player.SetInWater(m_IsInWater);

		//! update active surface
		m_SurfaceType = surfType;
	}
	
	// ENVIRONMENT
	// Returns amount of deg C air temperature should be lowered by, based on player's height above water level
	float GetTemperatureHeightCorrection()
	{
		float temperature_reduction = Math.Max(0, (m_PlayerHeightPos * GameConstants.ENVIRO_TEMPERATURE_HEIGHT_REDUCTION));
		return temperature_reduction;
	}
	
	float GetWindModifierPerSurface()
	{
		if (IsUnderRoofBuilding())
			return 0.0;

		return g_Game.ConfigGetFloat("CfgSurfaces " + m_SurfaceType + " windModifier");
	}
	
	float GetTemperature()
	{
		return m_EnvironmentTemperature;
	}
	
	// Calculates and return temperature of environment
	protected float GetEnvironmentTemperature()
	{
		float temperature;
		temperature = g_Game.GetMission().GetWorldData().GetBaseEnvTemperature();
		temperature += Math.AbsFloat(temperature * m_Clouds * GameConstants.ENVIRO_CLOUDS_TEMP_EFFECT);
		
		if (IsWaterContact())
		{
			temperature -= Math.AbsFloat(temperature * GameConstants.ENVIRO_WATER_TEMPERATURE_COEF);
		}
		
		if (IsInsideBuilding() || m_IsUnderRoofBuilding)
		{
			temperature += Math.AbsFloat(temperature * GameConstants.ENVIRO_TEMPERATURE_INSIDE_COEF);
		}
		else if (IsInsideVehicle())
		{
			temperature += Math.AbsFloat(temperature * GameConstants.ENVIRO_TEMPERATURE_INSIDE_VEHICLE_COEF);
		}
		else if (IsUnderRoof() && !m_IsUnderRoofBuilding)
		{
			temperature += Math.AbsFloat(temperature * GameConstants.ENVIRO_TEMPERATURE_UNDERROOF_COEF);
			temperature -= GameConstants.ENVIRO_TEMPERATURE_WIND_COEF * GetWindModifierPerSurface() * m_Wind;
		}
		else
		{
			temperature -= GameConstants.ENVIRO_TEMPERATURE_WIND_COEF * GetWindModifierPerSurface() * m_Wind;
			temperature -= Math.AbsFloat(temperature * m_Fog * GameConstants.ENVIRO_FOG_TEMP_EFFECT);
			temperature -= GetTemperatureHeightCorrection();
		}
		
		// incorporate temperature from temperature sources (buffer)
		if (Math.AbsFloat(m_UTSAverageTemperature) > 0.001)
		{
			temperature += m_UTSAverageTemperature;
		}
		
		return temperature;
	}	
		
	// Calculats wet/drying delta based on player's location and weather
	float GetWetDelta()
	{
		float wetDelta = 0;
		if ( IsWaterContact() )
		{
			//! player is getting wet by movement/swimming in water (+differentiate wetDelta by water level)
			if (m_WaterLevel >= WATER_LEVEL_HIGH)
			{
				wetDelta = 1;
			}
			else if (m_WaterLevel >= WATER_LEVEL_MID && m_WaterLevel < WATER_LEVEL_HIGH)
			{
				wetDelta = 0.66;
			}
			else if (m_WaterLevel >= WATER_LEVEL_LOW && m_WaterLevel < WATER_LEVEL_MID)
			{
				wetDelta = 0.66;
			}
			else if (m_WaterLevel >= WATER_LEVEL_NONE && m_WaterLevel < WATER_LEVEL_LOW)
			{
				wetDelta = 0.33;
			}
		}
		else if (IsRaining() && !IsInsideBuilding() && !IsUnderRoof() && !IsInsideVehicle())
		{
			//! player is getting wet from rain
			wetDelta = GameConstants.ENVIRO_WET_INCREMENT * GameConstants.ENVIRO_TICKS_TO_WETNESS_CALCULATION * (m_Rain) * (1 + (GameConstants.ENVIRO_WIND_EFFECT * m_Wind));
		}
		else
		{
			//! player is drying
			float tempEffect = Math.Max(m_PlayerHeat + GetEnvironmentTemperature(), 1.0);

			float weatherEffect = ((1 - (m_Fog * GameConstants.ENVIRO_FOG_DRY_EFFECT))) * (1 - (m_Clouds * GameConstants.ENVIRO_CLOUD_DRY_EFFECT));
			if (weatherEffect <= 0)
			{
				weatherEffect = 1.0;
			}
			
			wetDelta = -(GameConstants.ENVIRO_DRY_INCREMENT * weatherEffect * tempEffect);
			if (!IsInsideBuilding())
			{
				wetDelta *= 1 + (GameConstants.ENVIRO_WIND_EFFECT * m_Wind);
			}
		}

		return wetDelta;
	}

	// EXPOSURE
	// Each tick updates current entity member variables
	protected void CollectAndSetPlayerData()
	{
		vector playerPos = m_Player.GetPosition();
		m_PlayerHeightPos = playerPos[1];

		HumanCommandMove hcm = m_Player.GetCommand_Move();
		if (hcm)
		{
			m_PlayerSpeed = hcm.GetCurrentMovementSpeed();
		}

		m_PlayerHeat = GetPlayerHeat();
	}
	
	// Each tick updates current environment member variables
	protected void CollectAndSetEnvironmentData()
	{
		Weather weather	= g_Game.GetWeather();
		m_Rain			= weather.GetRain().GetActual();
		m_DayOrNight 	= g_Game.GetWorld().GetSunOrMoon();
		m_Fog 			= weather.GetFog().GetActual();
		m_Clouds 		= weather.GetOvercast().GetActual();
		m_Wind 			= m_WindAverageBuffer.Add(weather.GetWindSpeed() / weather.GetWindMaximumSpeed());

		SetEnvironmentTemperature();
	}
	
	void SetEnvironmentTemperature()
	{
		m_IsTempSet = true;
		m_EnvironmentTemperature = GetEnvironmentTemperature();
	}

	protected void ProcessWetnessByRain()
	{
		ProcessItemsWetness(m_SlotIdsComplete);
	}

	protected void ProcessWetnessByWaterLevel(float pWaterLevel)
	{
		// process attachments by water depth
		if (pWaterLevel >= WATER_LEVEL_HIGH)
		{
			//! complete
			ProcessItemsWetness(m_SlotIdsComplete);
		}
		else if (pWaterLevel >= WATER_LEVEL_MID && pWaterLevel < WATER_LEVEL_HIGH)
		{
			//! upper part
			ProcessItemsWetness(m_SlotIdsUpper);
		}
		else if (pWaterLevel >= WATER_LEVEL_LOW && pWaterLevel < WATER_LEVEL_MID)
		{
			//! bottom part
			ProcessItemsWetness(m_SlotIdsBottom);
		}
		else if (pWaterLevel >= WATER_LEVEL_NONE && pWaterLevel < WATER_LEVEL_LOW)
		{
			//! feet
			ProcessItemsWetness(m_SlotIdsLower);
		}
	}

	// Wets or dry items once in given time
	protected void ProcessItemsWetness(array<int> pSlotIds)
	{
		EntityAI attachment;
		
		int playerAttachmentCount = m_Player.GetInventory().AttachmentCount();
		
		LogDryWetProcess(string.Format("Environment :: ProcessItemsWetness (update interval=%1s)", GameConstants.ENVIRO_TICK_RATE));
		for (int attIdx = 0; attIdx < playerAttachmentCount; ++attIdx)
		{
			attachment = m_Player.GetInventory().GetAttachmentFromIndex(attIdx);
			if (attachment.IsItemBase())
			{
				int attachmentSlotsCount = attachment.GetInventory().GetSlotIdCount();
				for (int attachmentSlotId = 0; attachmentSlotId < attachmentSlotsCount; ++attachmentSlotId)
				{
					int attachmentSlot = attachment.GetInventory().GetSlotId(attachmentSlotId);
					for (int i = 0; i < pSlotIds.Count(); ++i)
					{
						if (attachmentSlot == pSlotIds.Get(i))
						{
							ApplyWetnessToItem(ItemBase.Cast(attachment));
							break;
						}
					}
				}
			}
		}
		
		if (m_Player.GetItemInHands())
			ApplyWetnessToItem(m_Player.GetItemInHands());

		LogDryWetProcess("==========");

	}

	protected void ProcessItemsDryness()
	{
		EntityAI attachment;
		ItemBase item;
		
		int attCount = m_Player.GetInventory().AttachmentCount();
		
		LogDryWetProcess(string.Format("Environment :: ProcessItemsDryness (update interval=%1s)", GameConstants.ENVIRO_TICK_RATE));
		EnvironmentDrynessData drynessData = new EnvironmentDrynessData();
		drynessData.m_UseTemperatureSources = m_HasTemperatureSources;

		if (m_HasTemperatureSources)
		{
			float distance = vector.Distance(m_UTemperatureSources[0].GetPosition(), m_Player.GetPosition());
			distance = Math.Max(distance, 0.1);
			drynessData.m_TemperatureSourceDistance = distance;
			LogDryWetProcess(string.Format("distance to heatsource: %1 m", distance));
		}
		
		for (int attIdx = 0; attIdx < attCount; attIdx++)
		{
			attachment = m_Player.GetInventory().GetAttachmentFromIndex(attIdx);
			if (attachment && attachment.IsItemBase())
			{
				item = ItemBase.Cast(attachment);
				if (item)
					ApplyDrynessToItemEx(item, drynessData);
			}
		}

		if (m_Player.GetItemInHands())
		{
			ApplyDrynessToItemEx(m_Player.GetItemInHands(), drynessData);
		}
		
		LogDryWetProcess("==========");
	}

	protected void ApplyWetnessToItem(ItemBase pItem)
	{
		if (pItem)
		{
			ItemBase parentItem;
			bool isParentWet 			= false;
			bool parentContainsLiquid 	= false;
			float liquidModifier 		= 1.0;
			InventoryLocation iLoc 		= new InventoryLocation();

			if (pItem.GetInventory().GetCurrentInventoryLocation(iLoc))
			{
				EntityAI parent = iLoc.GetParent();
				if (parent)
				{
					parentItem = ItemBase.Cast(parent);
					if (parentItem)
					{
						if (parentItem.GetWet() >= GameConstants.STATE_SOAKING_WET)
							isParentWet = true;

						if ((parentItem.GetLiquidType() != 0) && (parentItem.GetQuantity() > 0))
						{
							parentContainsLiquid = true;
							liquidModifier = pItem.GetLiquidModifierCoef();
						}
					}
					
					if ((pItem.GetWet() > m_ItemsWetnessMax) && (parent == m_Player))
						m_ItemsWetnessMax = pItem.GetWet();
				}
			}

			float wettingCoef = 0;

			if (!isParentWet && !parentContainsLiquid)
			{
				wettingCoef = GetWetDelta();
				LogDryWetProcess(string.Format("%1 (dry coef=%2/s, current wetness=%3) [normal]", pItem.GetDisplayName(), wettingCoef / GameConstants.ENVIRO_TICK_RATE, pItem.GetWet()), parentItem != null);
				pItem.AddWet(wettingCoef);
				pItem.AddTemperature(GameConstants.ENVIRO_TICK_RATE * GameConstants.TEMPERATURE_RATE_COOLING_PLAYER * liquidModifier);

				if (pItem.GetInventory().GetCargo())
				{
					int inItemCount = pItem.GetInventory().GetCargo().GetItemCount();
					for (int i = 0; i < inItemCount; i++)
					{
						ItemBase inItem;
						if (Class.CastTo(inItem, pItem.GetInventory().GetCargo().GetItem(i)))
							ApplyWetnessToItem(inItem);
					}
				}
				
				int attCount = pItem.GetInventory().AttachmentCount();
				if (attCount > 0)
				{
					for (int attIdx = 0; attIdx < attCount; attIdx++)
					{
						EntityAI attachment = pItem.GetInventory().GetAttachmentFromIndex(attIdx);
						ItemBase itemAtt = ItemBase.Cast(attachment);
						if (itemAtt)
							ApplyWetnessToItem(itemAtt);
					}
				}
			}

			if (parentContainsLiquid)
			{
				wettingCoef = GetWetDelta() * liquidModifier;
				LogDryWetProcess(string.Format("%1 (dry coef=%2/s, current wetness=%3) [parent contains liquid]", pItem.GetDisplayName(), wettingCoef / GameConstants.ENVIRO_TICK_RATE, pItem.GetWet()), parentItem != null);
				pItem.AddWet(wettingCoef);
				pItem.AddTemperature(GameConstants.ENVIRO_TICK_RATE * GameConstants.TEMPERATURE_RATE_COOLING_PLAYER * pItem.GetLiquidModifierCoef());
			}
	
			if (isParentWet)
			{
				if (pItem.GetWet() < parentItem.GetWet())
				{
					wettingCoef = GetWetDelta() * pItem.GetTransferableWetnessCoef();
					LogDryWetProcess(string.Format("%1 (dry coef=%2/s, current wetness=%3) [parent wet]", pItem.GetDisplayName(), wettingCoef / GameConstants.ENVIRO_TICK_RATE, pItem.GetWet()), parentItem != null);
					pItem.AddWet(wettingCoef);
				}
	
				pItem.AddTemperature(GameConstants.ENVIRO_TICK_RATE * GameConstants.TEMPERATURE_RATE_COOLING_PLAYER * 3.5);
			}
		}
	}
	
	protected void ApplyDrynessToItem(ItemBase pItem)
	{
		EnvironmentDrynessData drynessData = new EnvironmentDrynessData();
		ApplyDrynessToItemEx(pItem, drynessData);
	}
	
	protected void ApplyDrynessToItemEx(ItemBase pItem, EnvironmentDrynessData pDrynessData)
	{
		if (pItem)
		{
			float dryingIncrement = pItem.GetDryingIncrements(EDryingIncrementCategory.PLAYER);
			if (pDrynessData.m_UseTemperatureSources)
				dryingIncrement = pItem.GetDryingIncrements(EDryingIncrementCategory.PLAYER_FIREPLACE);
			
			ItemBase parentItem;
			bool isParentWet 			= false;
			bool parentContainsLiquid 	= false;
	
			InventoryLocation iLoc = new InventoryLocation();
			if (pItem.GetInventory().GetCurrentInventoryLocation(iLoc))
			{
				EntityAI parent = iLoc.GetParent();
				if (parent)
				{
					parentItem = ItemBase.Cast(parent);
					if (parentItem)
					{
						if (parentItem.GetWet() >= GameConstants.STATE_DAMP)
							isParentWet = true;
	
						if ((parentItem.GetLiquidType() != 0) && (parentItem.GetQuantity() > 0))
							parentContainsLiquid = true;
					}
					
					if ((pItem.GetWet() > m_ItemsWetnessMax) && (parent == m_Player))
					{
						m_ItemsWetnessMax = pItem.GetWet();
					}
				}
			}
	
			float dryingCoef = 0;
			
			if (!isParentWet && !parentContainsLiquid)
			{
				dryingCoef = (-1 * GameConstants.ENVIRO_TICK_RATE * dryingIncrement) / pDrynessData.m_TemperatureSourceDistance;
				if (pItem.GetWet() >= GameConstants.STATE_DAMP)
				{
					LogDryWetProcess(string.Format("%1 (dry coef=%2/s, current wetness=%3) [normal]", pItem.GetDisplayName(), dryingCoef / GameConstants.ENVIRO_TICK_RATE, pItem.GetWet()), parentItem != null);
					pItem.AddWet(dryingCoef);
					pItem.AddTemperature(GameConstants.ENVIRO_TICK_RATE * GameConstants.TEMPERATURE_RATE_COOLING_PLAYER);
				}
				
				if (pItem.GetInventory().GetCargo())
				{
					int inItemCount = pItem.GetInventory().GetCargo().GetItemCount();
					for (int i = 0; i < inItemCount; i++)
					{
						ItemBase inItem;
						if (Class.CastTo(inItem, pItem.GetInventory().GetCargo().GetItem(i)))
							ApplyDrynessToItemEx(inItem, pDrynessData);
					}
				}
				
				int attCount = pItem.GetInventory().AttachmentCount();
				if (attCount > 0)
				{
					for (int attIdx = 0; attIdx < attCount; attIdx++)
					{
						EntityAI attachment = pItem.GetInventory().GetAttachmentFromIndex(attIdx);
						ItemBase itemAtt;
						if (ItemBase.CastTo(itemAtt, attachment))
							ApplyDrynessToItemEx(itemAtt, pDrynessData);
					}
				}
			}
			
			if (parentContainsLiquid)
			{
				//! adds wetness to item inside parent item containing liquid
				dryingCoef = (GameConstants.ENVIRO_TICK_RATE * dryingIncrement * pItem.GetLiquidModifierCoef()) / pDrynessData.m_TemperatureSourceDistance;
				LogDryWetProcess(string.Format("%1 (dry coef=%2/s, current wetness=%3) [parent contains liquid]", pItem.GetDisplayName(), dryingCoef / GameConstants.ENVIRO_TICK_RATE, pItem.GetWet()), parentItem != null);
				pItem.AddWet(dryingCoef);
				pItem.AddTemperature(GameConstants.ENVIRO_TICK_RATE * GameConstants.TEMPERATURE_RATE_COOLING_PLAYER * pItem.GetLiquidModifierCoef());
			}
	
			if (isParentWet)
			{
				if (pItem.GetWet() < parentItem.GetWet())
				{
					//! adds wetness to item inside wet parent item
					dryingCoef = (GameConstants.ENVIRO_TICK_RATE * dryingIncrement * pItem.GetTransferableWetnessCoef()) / pDrynessData.m_TemperatureSourceDistance;
					LogDryWetProcess(string.Format("%1 (dry coef=%2/s, current wetness=%3) [parent wet]", pItem.GetDisplayName(), dryingCoef / GameConstants.ENVIRO_TICK_RATE, pItem.GetWet()), parentItem != null);
					pItem.AddWet(dryingCoef);
				}
	
				pItem.AddTemperature(GameConstants.ENVIRO_TICK_RATE * GameConstants.TEMPERATURE_RATE_COOLING_PLAYER * 3.5);
			}
		}
	}

	// HEAT COMFORT
	//! Calculates and process temperature of items
	protected void ProcessItemsHeat()
	{
		float hcHead, hcBody, hcFeet;	//! Heat Comfort
		float hHead, hBody, hFeet;		//! Heat (from items);
		
		float heatComfortAvg;
		float heatAvg;

		BodyPartHeatProperties(m_HeadParts, GameConstants.ENVIRO_HEATCOMFORT_HEADPARTS_WEIGHT, hcHead, hHead);
		BodyPartHeatProperties(m_BodyParts, GameConstants.ENVIRO_HEATCOMFORT_BODYPARTS_WEIGHT, hcBody, hBody);
		BodyPartHeatProperties(m_FeetParts, GameConstants.ENVIRO_HEATCOMFORT_FEETPARTS_WEIGHT, hcFeet, hFeet);

		heatComfortAvg = (hcHead + hcBody + hcFeet) / 3;
		heatAvg = (hHead + hBody + hFeet) / 3;
		heatAvg = heatAvg * GameConstants.ENVIRO_ITEM_HEAT_TRANSFER_COEF;
		
		// heat buffer
		float applicableHB = 0.0;
		if (m_UTSAverageTemperature < 0.001)
		{
			applicableHB = m_Player.GetStatHeatBuffer().Get() / 30.0;
			if (applicableHB > 0.0)
			{
				if (m_HeatBufferTimer > 1.0)
				{
					m_Player.GetStatHeatBuffer().Add(Math.Min(EnvTempToCoef(m_EnvironmentTemperature), -0.1) * GameConstants.ENVIRO_PLAYER_HEATBUFFER_DECREASE);
				}
				else
				{
					m_HeatBufferTimer += GameConstants.ENVIRO_PLAYER_HEATBUFFER_TICK;
				}
			}
			else
			{
				m_HeatBufferTimer = 0.0;
			}			
		}
		else
		{
			applicableHB = m_Player.GetStatHeatBuffer().Get() / 30.0;
			if (m_HeatComfort > PlayerConstants.THRESHOLD_HEAT_COMFORT_MINUS_WARNING)
			{
				m_Player.GetStatHeatBuffer().Add(GameConstants.ENVIRO_PLAYER_HEATBUFFER_INCREASE);
				m_HeatBufferTimer = 0.0;
			}
			else
			{
				m_HeatBufferTimer = 0.0;
			}
		}
		
		m_HeatComfort = (heatComfortAvg + heatAvg + (GetPlayerHeat() / 100)) + EnvTempToCoef(m_EnvironmentTemperature);
		if ((m_HeatComfort + applicableHB) < (PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_WARNING - 0.01))
		{
			m_HeatComfort += applicableHB;
		}
		else
		{
			if (m_HeatComfort <= (PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_WARNING - 0.01))
			{
				m_HeatComfort = PlayerConstants.THRESHOLD_HEAT_COMFORT_PLUS_WARNING - 0.01;
			}
		}

		m_HeatComfort = Math.Clamp(m_HeatComfort, m_Player.GetStatHeatComfort().GetMin(), m_Player.GetStatHeatComfort().GetMax());
		
		m_Player.GetStatHeatComfort().Set(m_HeatComfort);
	}
	
	//! DEPRECATED
	protected bool OverridenHeatComfort(out float value);
	
	protected float EnvTempToCoef(float pTemp)
	{
		if (pTemp >= GameConstants.ENVIRO_HIGH_TEMP_LIMIT)
		{
			return 1;
		}
		
		if (pTemp <= GameConstants.ENVIRO_LOW_TEMP_LIMIT)
		{
			return -1;
		}

		return (pTemp - GameConstants.ENVIRO_PLAYER_COMFORT_TEMP) / GameConstants.ENVIRO_TEMP_EFFECT_ON_PLAYER;
	}
	
	//! returns weighted avg heat comfort for bodypart
	protected void BodyPartHeatProperties(array<int> pBodyPartIds, float pCoef, out float pHeatComfort, out float pHeat)
	{
		int attCount;
		
		EntityAI attachment;
		ItemBase item;
		
		pHeatComfort = -1;
		attCount = m_Player.GetInventory().AttachmentCount();
		
		for (int attIdx = 0; attIdx < attCount; attIdx++)
		{
			attachment = m_Player.GetInventory().GetAttachmentFromIndex(attIdx);
			if (attachment.IsClothing())
			{
				item = ItemBase.Cast(attachment);
				int attachmentSlot = attachment.GetInventory().GetSlotId(0);

				//! go through all body parts we've defined for that zone (ex.: head, body, feet)
				for (int i = 0; i < pBodyPartIds.Count(); i++)
				{
					if (attachmentSlot == pBodyPartIds.Get(i))
					{
						float heatIsoMult = 1.0;
						if (attachmentSlot == InventorySlots.VEST)
						{
							heatIsoMult = GameConstants.ENVIRO_HEATISOLATION_VEST_WEIGHT;
						}

						if (attachmentSlot == InventorySlots.BACK)
						{
							heatIsoMult = GameConstants.ENVIRO_HEATISOLATION_BACK_WEIGHT;
						}

						pHeatComfort += heatIsoMult * MiscGameplayFunctions.GetCurrentItemHeatIsolation(item);
						
						// go through any attachments and cargo (only current level, ignore nested containers - they isolate)
						int inAttCount = item.GetInventory().AttachmentCount();
						if (inAttCount > 0)
						{
							for (int inAttIdx = 0; inAttIdx < inAttCount; inAttIdx++)
							{
								EntityAI inAttachment = item.GetInventory().GetAttachmentFromIndex(inAttIdx);
								ItemBase itemAtt = ItemBase.Cast(inAttachment);
								if (itemAtt)
								{
									pHeat += itemAtt.GetTemperature();
								}
							}
						}
						if (item.GetInventory().GetCargo())
						{
							int inItemCount = item.GetInventory().GetCargo().GetItemCount();
							
							for (int j = 0; j < inItemCount; j++)
							{
								ItemBase inItem;
								if (Class.CastTo(inItem, item.GetInventory().GetCargo().GetItem(j)))
								{
									pHeat += inItem.GetTemperature();
								}
							}
						}
					}
				}
			}
		}

		pHeatComfort = (pHeatComfort / pBodyPartIds.Count()) * pCoef;
		pHeat = (pHeat / pBodyPartIds.Count()) * pCoef;
	}
	
	protected void GatherTemperatureSources()
	{
		m_UTemperatureSources.Clear();

		array<Object> nearestObjects = new array<Object>;
		GetGame().GetObjectsAtPosition(m_Player.GetPosition(), GameConstants.ENVIRO_TEMP_SOURCES_LOOKUP_RADIUS, nearestObjects, null);

		foreach (Object nearestObject : nearestObjects)
		{
			EntityAI ent = EntityAI.Cast(nearestObject);
			if (ent && ent.IsUniversalTemperatureSource() && ent != m_Player)
			{
				//! next temp source is too far
				if (vector.DistanceSq(m_Player.GetPosition(), ent.GetPosition()) > Math.SqrFloat(ent.GetUniversalTemperatureSource().GetMaxRange()))
					continue;

				m_UTemperatureSources.Insert(ent.GetUniversalTemperatureSource());
			}
		}

		if (m_Player.GetItemInHands() && m_Player.GetItemInHands().IsUniversalTemperatureSource())
			m_UTemperatureSources.Insert(m_Player.GetItemInHands().GetUniversalTemperatureSource());
	}
	
	protected void ProcessTemperatureSources()
	{
		if (m_UTemperatureSources.Count() == 0)
		{
			m_HasTemperatureSources = false;
			m_UTSAverageTemperature = m_UTSAverageTemperatureBuffer.Add(0);
			return;
		}

		array<float> utsTemperatures = new array<float>();
		
		// get temperature from the source (based on distance), save it for min/max filtering
		foreach (UTemperatureSource tempSource : m_UTemperatureSources)
			utsTemperatures.Insert(CalcTemperatureFromTemperatureSource(tempSource));

		float min = MiscGameplayFunctions.GetMinValue(utsTemperatures);
		float max = MiscGameplayFunctions.GetMaxValue(utsTemperatures);
		
		if (max > 0 && min < 0)
		{
			//! adds average of 2 most significat sources to buffer
			m_UTSAverageTemperature = m_UTSAverageTemperatureBuffer.Add((max + min) * 0.5);
		}
		else
		{
			m_UTSAverageTemperature = m_UTSAverageTemperatureBuffer.Add(max);
		}
		
		m_HasTemperatureSources = true;
	}
	
	float GetUniversalSourcesTemperageAverage()
	{
		return m_UTSAverageTemperature;
	}
	
	float CalcTemperatureFromTemperatureSource(notnull UTemperatureSource uts)
	{
		float distance 		= vector.Distance(m_Player.GetPosition(), uts.GetPosition());
		distance 			= Math.Max(distance, 0.1);	//min distance cannot be 0 (division by zero)
		float temperature	= 0;

		//Debug.Log(string.Format("CalcTemperatureFromTemperatureSource::distance: %1", distance), "Environment");
		
		//! heat transfer through air to player (env temperature)
		if (distance > uts.GetFullRange())
		{
			float distFactor = 1 - (distance / uts.GetMaxRange());
			distFactor = Math.Max(distFactor, 0.0); //! dist factor minimum should be at 0
			temperature = uts.GetTemperature() * distFactor;
			//temperature = Math.Clamp(temperature, uts.GetTemperatureMin(), uts.GetTemperatureMax());
			//Debug.Log(string.Format("CalcTemperatureFromTemperatureSource::distFactor: %1", distFactor), "Environment");
			//Debug.Log(string.Format("CalcTemperatureFromTemperatureSource::temperatureXX: %1", temperature), "Environment");
		}
		else
		{
			temperature = uts.GetTemperature();
		}

		//Debug.Log(string.Format("CalcTemperatureFromTemperatureSource::temperature: %1", temperature), "Environment");

		return temperature;
	}

	//! debug
#ifdef DIAG_DEVELOPER
	EnvDebugData GetEnvDebugData()
	{
		EnvDebugData data = new EnvDebugData();
		data.Synch(this, m_Player);
		return data;
	}
	
	void ShowEnvDebugPlayerInfo(bool enabled)
	{
		EnvDebugData data = GetEnvDebugData();
		DisplayEnvDebugPlayerInfo(enabled, data);
	}
	
	static void DisplayEnvDebugPlayerInfo(bool enabled, EnvDebugData data)
	{
		int windowPosX = 10;
		int windowPosY = 200;

		Object obj;

		DbgUI.Begin("Player stats", windowPosX, windowPosY);
		if ( enabled )
		{
			DbgUI.Text(string.Format("Heat comfort: %1", data.m_PlayerData.m_HeatComfort));
			DbgUI.Text(string.Format("Inside: %1 (%2)", data.m_PlayerData.m_Inside, data.m_PlayerData.m_Surface));
			DbgUI.Text(string.Format("Under roof: %1 (%2)", data.m_PlayerData.m_UnderRoof, data.m_PlayerData.m_UnderRoofTimer));
			if ( data.m_PlayerData.m_WaterLevel > 0 )
			{
				DbgUI.Text(string.Format("Water Level: %1", data.m_PlayerData.m_WaterLevel));
			}
			
		}
		DbgUI.End();
		
		DbgUI.Begin("Weather stats:", windowPosX, windowPosY + 200);
		if ( enabled )
		{
			DbgUI.Text(string.Format("Env temperature (base): %1", data.m_MiscData.m_TemperatureBase));
			DbgUI.Text(string.Format("Env temperature (modfied): %1", data.m_MiscData.m_TemperatureModified));
			DbgUI.Text(string.Format("Wind: %1 (x%2)", data.m_WeatherData.m_Wind, data.m_WeatherData.m_WindModifier));
			DbgUI.Text(string.Format("Rain: %1", data.m_WeatherData.m_Rain));
			DbgUI.Text(string.Format("Day/Night (1/0): %1", data.m_MiscData.m_DayOrNight));
			DbgUI.Text(string.Format("Fog: %1", data.m_WeatherData.m_Fog));
			DbgUI.Text(string.Format("Clouds: %1", data.m_WeatherData.m_Clouds));
			DbgUI.Text(string.Format("Height: %1", data.m_MiscData.m_Height));
			DbgUI.Text(string.Format("Wet delta: %1", data.m_MiscData.m_WetDelta));
		}
		DbgUI.End();
	}
	
	void FillDebugWeatherData(EnvDebugWeatherData data)
	{
		data.m_Wind = m_Wind;
		data.m_WindModifier = GetWindModifierPerSurface();
		data.m_Rain = m_Rain;
		data.m_Fog = m_Fog;
		data.m_Clouds = m_Clouds;
	}
#endif

	string GetDebugMessage()
	{
		string message;		
		message += "Player stats";
		message += "\nHeat comfort: " + m_HeatComfort.ToString();
		message += "\nInside: " + IsInsideBuilding().ToString() + " (" + m_Player.GetSurfaceType() + ")";
		message += "\nUnder roof: " + m_IsUnderRoof.ToString() + " (" + GetNextRoofCheck() + ")";
		if (IsWaterContact() && m_WaterLevel > WATER_LEVEL_NONE)
		{
			message += "\nWater Level: " + m_WaterLevel;
		}

		message += "\n\nWeather stats";
		message += "\nEnv temperature (base): " + g_Game.GetMission().GetWorldData().GetBaseEnvTemperature().ToString();
		message += "\nEnv temperature (modified): " + m_EnvironmentTemperature.ToString();
		message += "\nWind: " + m_Wind.ToString() + " (x" + GetWindModifierPerSurface() + ")";
		message += "\nRain: " + m_Rain.ToString();
		message += "\nDay/Night (1/0): " + m_DayOrNight.ToString();
		message += "\nFog: " + m_Fog.ToString();
		message += "\nClouds: " + m_Clouds.ToString();
		message += "\nHeight: " + GetTemperatureHeightCorrection().ToString();
		message += "\nWet delta: " + GetWetDelta().ToString();
		
		return message;
	}
	
	int GetNextRoofCheck()
	{
		return (GameConstants.ENVIRO_TICK_ROOF_RC_CHECK - m_RoofCheckTimer) + 1;
	}
	
	float GetWaterLevel()
	{
		if (IsWaterContact() && m_WaterLevel > WATER_LEVEL_NONE)
		{
			return m_WaterLevel;
		}
		else
		{
			return 0;
		}
	}
	
	float GetDayOrNight()
	{
		return m_DayOrNight;
	}
	
	private void LogDryWetProcess(string message, bool indented = false)
	{
		#ifdef DIAG_DEVELOPER
		if (m_DebugLogDryWet)
		{
			string indentation = "";
			if (indented)
				indentation = "|--";

			Debug.Log(string.Format("%1 %2", indentation, message));
		}
		#endif
	}
}

class EnvironmentDrynessData
{
	bool m_UseTemperatureSources 		= false;
	float m_TemperatureSourceDistance	= 1.0;
}

#ifdef DIAG_DEVELOPER
class EnvDebugPlayerData : Param
{
	float 	m_HeatComfort;
	bool 	m_Inside;
	string 	m_Surface;
	bool 	m_UnderRoof;
	int 	m_UnderRoofTimer;
	float 	m_WaterLevel;
	
	void Synch(Environment env, PlayerBase player)
	{
		m_HeatComfort = player.GetStatHeatComfort().Get();
		m_Inside = env.IsInsideBuilding();
		m_Surface = player.GetSurfaceType();
		m_UnderRoof = env.IsUnderRoof();
		m_UnderRoofTimer = env.GetNextRoofCheck();
		m_WaterLevel = env.GetWaterLevel();
	}
	
	override bool Serialize(Serializer ctx)
	{
		return (
		ctx.Write(m_HeatComfort) && ctx.Write(m_Inside) && ctx.Write(m_Surface) && ctx.Write(m_UnderRoof) && ctx.Write(m_UnderRoofTimer) && ctx.Write(m_WaterLevel));
	}
	
	override bool Deserializer(Serializer ctx)
	{
		return ctx.Read(m_HeatComfort) && ctx.Read(m_Inside) && ctx.Read(m_Surface) && ctx.Read(m_UnderRoof) && ctx.Read(m_UnderRoofTimer) && ctx.Read(m_WaterLevel);
	}
}

class EnvDebugMiscData : Param
{
	float 	m_TemperatureBase;
	float 	m_TemperatureModified;
	float 	m_DayOrNight;
	float 	m_Height;
	float 	m_WetDelta;
	
	void Synch(Environment env)
	{
		m_TemperatureBase = g_Game.GetMission().GetWorldData().GetBaseEnvTemperature();
		m_TemperatureModified = env.GetTemperature();
		m_DayOrNight = env.GetDayOrNight();
		m_Height = env.GetTemperatureHeightCorrection();
		m_WetDelta = env.GetWetDelta();
	}
	
	override bool Serialize(Serializer ctx)
	{
		return ctx.Write(m_TemperatureBase) && ctx.Write(m_TemperatureModified) && ctx.Write(m_DayOrNight) && ctx.Write(m_Height) && ctx.Write(m_WetDelta);
	}
	
	override bool Deserializer(Serializer ctx)
	{
		return ctx.Read(m_TemperatureBase) && ctx.Read(m_TemperatureModified) && ctx.Read(m_DayOrNight) && ctx.Read(m_Height) && ctx.Read(m_WetDelta);
	}
}

class EnvDebugWeatherData : Param
{
	float 	m_Wind;
	float 	m_WindModifier;
	float 	m_Rain;
	float 	m_Fog;
	float 	m_Clouds;
	
	void Synch(Environment env)
	{
		env.FillDebugWeatherData(this);
	}
	
	override bool Serialize(Serializer ctx)
	{
		return ctx.Write(m_Wind) && ctx.Write(m_WindModifier) && ctx.Write(m_Rain) && ctx.Write(m_Fog) && ctx.Write(m_Clouds);
	}
		
	override bool Deserializer(Serializer ctx)
	{
		return ctx.Read(m_Wind) && ctx.Read(m_WindModifier) && ctx.Read(m_Rain) && ctx.Read(m_Fog) && ctx.Read(m_Clouds);
	}
}

class EnvDebugData : Param
{
	ref EnvDebugPlayerData 		m_PlayerData = new EnvDebugPlayerData();
	ref EnvDebugMiscData 		m_MiscData = new EnvDebugMiscData();
	ref EnvDebugWeatherData 	m_WeatherData = new EnvDebugWeatherData();
	
	void Synch(Environment env, PlayerBase player)
	{
		m_PlayerData.Synch(env, player);
		m_MiscData.Synch(env);
		m_WeatherData.Synch(env);
	}
	
	override bool Serialize(Serializer ctx)
	{
		return m_PlayerData.Serialize(ctx) && m_MiscData.Serialize(ctx) && m_WeatherData.Serialize(ctx);
	}
		
	override bool Deserializer(Serializer ctx)
	{
		return m_PlayerData.Deserializer(ctx) && m_MiscData.Deserializer(ctx) && m_WeatherData.Deserializer(ctx);
	}
}
#endif