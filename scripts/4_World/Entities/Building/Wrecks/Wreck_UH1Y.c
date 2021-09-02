class Wreck_UH1Y extends House
{
	Particle m_ParticleEfx;
	
	void Wreck_UH1Y()
	{
		if ( !GetGame().IsMultiplayer() || GetGame().IsClient() )
		{
			m_ParticleEfx = Particle.PlayOnObject(ParticleList.SMOKING_HELI_WRECK, this, Vector(-0.5, 0, -1.0));
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
		Param2<bool, vector> playSound = new Param2<bool, vector>(true, GetPosition() );
		array<ref Param> params = new array<ref Param>;
		params.Insert(playSound);
		GetGame().RPC( null, ERPCs.RPC_SOUND_HELICRASH, params, true );
	}
}