class GasMask_Filter extends ItemBase
{
	override void OnQuantityChanged(float delta)
	{
		super.OnQuantityChanged(delta);
		
		if (GetQuantity() == 0)
		{
			SetHealth("","",0);
		}
	}
	
	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
	{
		if (GetGame().IsServer())
		{
			if( newLevel == GameConstants.STATE_RUINED )
			{
				SetQuantity(0);
			}
		}
	}
}