class WeldingMask extends ClothingBase
{
	override array<int> GetEffectWidgetTypes()
	{
		return {EffectWidgetsTypes.HELMET_OCCLUDER,EffectWidgetsTypes.HELMET_BREATH};
	}
	
	override bool CanPutAsAttachment( EntityAI parent )
	{
		if(!super.CanPutAsAttachment(parent)) {return false;}
		bool is_mask_only = false;
		
		if ( parent.FindAttachmentBySlotName( "Mask" ) )
		{
			is_mask_only = parent.FindAttachmentBySlotName( "Mask" ).ConfigGetBool( "noHelmet" );
		}
		
		if ( ( GetNumberOfItems() == 0 || !parent || parent.IsMan() ) && !is_mask_only )
		{
			return true;
		}
		return false;
	}
		
	override int GetGlassesEffectID()
	{
		return PPERequesterBank.REQ_GLASSESWELDING;
	}
}