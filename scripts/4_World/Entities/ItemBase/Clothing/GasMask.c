class GasMask extends MaskBase
{
	override bool CanPutAsAttachment( EntityAI parent )
	{
		if (!super.CanPutAsAttachment(parent)) {return false;}
		bool headgear_present = false;
		
		if ( parent.FindAttachmentBySlotName( "Headgear" ) )
		{
			headgear_present = parent.FindAttachmentBySlotName( "Headgear" ).ConfigGetBool( "noMask" );
		}
		
		if ( ( GetNumberOfItems() == 0 || !parent || parent.IsMan() ) && !headgear_present )
		{
			return true;
		}
		return false;
	}
	
	override bool IsObstructingVoice()
	{
		return true;
	}
	
	override int GetVoiceEffect()
	{
		return VoiceEffectObstruction;
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
