class DesignerGlasses extends Clothing 
{
	override bool CanPutAsAttachment( EntityAI parent )
	{
		if(!super.CanPutAsAttachment(parent)) {return false;}
		return CanWearUnderMask( parent );
	}
	
	override int GetGlassesEffectID()
	{
		return PPERequesterBank.REQ_GLASSESDESIGNER;
	}
};