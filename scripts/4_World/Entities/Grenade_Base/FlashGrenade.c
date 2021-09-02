class FlashGrenade extends Grenade_Base
{
	const float FX_RANGE_MAX_MULT = 1.0;
	
	override void OnExplosionEffects(Object source, Object directHit, int componentIndex, string surface, vector pos, vector surfNormal, float energyFactor, float explosionFactor, bool isWater, string ammoType)
	{
		super.OnExplosionEffects(source, directHit, componentIndex, surface, pos, surfNormal, energyFactor, explosionFactor, isWater, ammoType);

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());

		if ( player )
		{
			bool visual = false;
			vector headPos = player.GetDamageZonePos("Head"); // animated position in the middle of the zone
			
			// get grenade range
			string grenade_path = "cfgAmmo " + ammoType + " indirectHitRange";
			float ammo_kill_range = GetGame().ConfigGetFloat(grenade_path);
			float ammo_max_range_mult = 4.0;
			
			grenade_path = "cfgAmmo " + ammoType + " indirectHitRangeMultiplier";
			if (GetGame().ConfigIsExisting(grenade_path))
			{
				ammo_max_range_mult = GetGame().ConfigGetFloat(grenade_path);
				if (ammo_max_range_mult < 1.0) //values less than 1.0 make no sense
				{
					ammo_max_range_mult = 1.0;
				}
			}
			float ammo_max_range = ammo_kill_range * ammo_max_range_mult;
			ammo_max_range = Math.Clamp( ammo_max_range * FX_RANGE_MAX_MULT,ammo_kill_range,ammo_max_range );
			
			float dist = vector.Distance(headPos, pos);
			float dist_sq = vector.DistanceSq(headPos, pos);
			float radius_sq_max = Math.SqrFloat(ammo_max_range);
			
			if ( dist_sq <= radius_sq_max )
			{
				// ignore collisions with parent if fireplace
				InventoryItem invItem = InventoryItem.Cast( source );
				EntityAI parent = invItem.GetHierarchyParent();
				array<Object> excluded = new array<Object>;
				
				if (!parent || !parent.IsFireplace())
					parent = null;
				else if (parent)
					excluded.Insert(parent);
				
				array<ref RaycastRVResult> results = new array<ref RaycastRVResult>;
				excluded.Insert(this); //Ignore self for visibility check
				
				//There shouldn't be cases justifying we go further than first entry (if in fireplace, self does not impact)
				RaycastRVParams rayParams = new RaycastRVParams(pos, headPos, excluded[0]);
				DayZPhysics.RaycastRVProxy(rayParams, results, excluded);
				
				//If player is not first index, object is between player and grenade
				if (PlayerBase.Cast(results[0].obj))
				{
					if ( MiscGameplayFunctions.IsPlayerOrientedTowardPos(player, pos, 60) )
					{
						visual = true;
					}
					
					float duration_coef;
					if (ammo_max_range == ammo_kill_range)
					{
						duration_coef = 1.0; //edge case, landed right on the edge
					}
					duration_coef = 1 - ((dist - ammo_kill_range) / (ammo_max_range - ammo_kill_range));
					duration_coef = Math.Clamp(duration_coef,0.1,100.0);
					
					float effect_coef = duration_coef;
					
					player.OnPlayerReceiveFlashbangHitStart(visual);
					player.GetFlashbangEffect().SetupFlashbangValues(duration_coef,effect_coef,effect_coef);
				}
			}
		}
	}

	void FlashGrenade()
	{
		SetAmmoType("FlashGrenade_Ammo");
		SetFuseDelay(2);
		SetGrenadeType(EGrenadeType.ILLUMINATING);
		SetParticleExplosion(ParticleList.GRENADE_M84);
	}
	
	protected override void CreateLight()
	{
		m_Light = GrenadeLight.Cast( ScriptedLightBase.CreateLight( FlashGrenadeLight, GetPosition()) );
	}

	void ~FlashGrenade() {}
}