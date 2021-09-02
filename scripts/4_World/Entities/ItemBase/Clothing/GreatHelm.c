class GreatHelm extends ClothingBase
{
	override array<int> GetEffectWidgetTypes()
	{
		return {EffectWidgetsTypes.HELMET_OCCLUDER/*,EffectWidgetsTypes.HELMET_BREATH*/};
	}
	
	override bool CanPutAsAttachment( EntityAI parent )
	{
		if(!super.CanPutAsAttachment(parent)) {return false;}
		
		if ( parent.FindAttachmentBySlotName( "Mask" ) )
		{
			return false;
		}
		
		if ( GetNumberOfItems() == 0 || !parent || parent.IsMan() )
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
}