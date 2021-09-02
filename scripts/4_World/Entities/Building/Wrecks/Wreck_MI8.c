//New russian helicopter crash site
class Wreck_Mi8_Crashed extends House
{
	Particle m_ParticleEfx;
	
	void Wreck_Mi8_Crashed()
	{
		if ( !GetGame().IsMultiplayer() || GetGame().IsClient() )
		{
			m_ParticleEfx = Particle.PlayOnObject(ParticleList.SMOKING_HELI_WRECK, this, Vector(2, 0, -5));
		}
	}
	
	override void EEInit()
	{
		//Setup for local sound tests
		#ifdef DEVELOPER
		if ( !GetGame().IsMultiplayer() )
			SEffectManager.PlaySound( "HeliCrash_Distant_SoundSet", GetPosition(), 0.1, 0.1 );
		#endif
	}
	
	override void EEDelete(EntityAI parent)
	{
		if ( !GetGame().IsMultiplayer() || GetGame().IsClient() )
		{
			if ( m_ParticleEfx )
				m_ParticleEfx.Stop();
		}
	}
	
	override void EEOnCECreate()
	{
		Param2<bool, vector> playSound = new Param2<bool, vector>(true, GetPosition());
		array<ref Param> params = new array<ref Param>;
		params.Insert(playSound);
		GetGame().RPC( null, ERPCs.RPC_SOUND_HELICRASH, params, true );
	}
}

//Old Russian helicopter crash site
class Wreck_Mi8 extends House
{
	void Wreck_Mi8()
	{
		//I'll leave this here if we want to add particles to MI-8
	}
	
	override void EEInit()
	{
		//Setup for local sound tests
		#ifdef DEVELOPER
		if ( !GetGame().IsMultiplayer() )
			SEffectManager.PlaySound( "HeliCrash_Distant_SoundSet", GetPosition(), 0.1, 0.1 );
		#endif
	}
	
	override void EEOnCECreate()
	{
		Param2<bool, vector> playSound = new Param2<bool, vector>(true, GetPosition());
		array<ref Param> params = new array<ref Param>;
		params.Insert(playSound);
		GetGame().RPC( null, ERPCs.RPC_SOUND_HELICRASH, params, true );
	}
}