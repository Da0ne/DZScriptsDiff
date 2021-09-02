class DarkMotoHelmet_ColorBase extends HelmetBase
{	
	override array<int> GetEffectWidgetTypes()
	{
		return {EffectWidgetsTypes.MOTO_OCCLUDER,EffectWidgetsTypes.MOTO_BREATH};
	}
	
	override bool CanPutAsAttachment( EntityAI parent )
	{
		if (!super.CanPutAsAttachment(parent)) {return false;}
		
		if ( parent.FindAttachmentBySlotName( "Mask" ) )
		{
			return false;
		}
		
		if ( ( GetNumberOfItems() == 0 || !parent || parent.IsMan() ) )
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
		
	override int GetGlassesEffectID()
	{
		return PPERequesterBank.REQ_MOTOHELMETBLACK;		
	}
}

class DarkMotoHelmet_Black extends DarkMotoHelmet_ColorBase {};
class DarkMotoHelmet_Grey extends DarkMotoHelmet_ColorBase {};
class DarkMotoHelmet_Green extends DarkMotoHelmet_ColorBase {};
class DarkMotoHelmet_Lime extends DarkMotoHelmet_ColorBase {};
class DarkMotoHelmet_Blue extends DarkMotoHelmet_ColorBase {};
class DarkMotoHelmet_Red extends DarkMotoHelmet_ColorBase {};
class DarkMotoHelmet_White extends DarkMotoHelmet_ColorBase {};