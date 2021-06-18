class ChristmasTree extends House
{
	XmasTreeLight m_TreeLight;
	
	void ChristmasTree()
	{
		if ( !GetGame().IsMultiplayer() || GetGame().IsClient() )
		{
			m_TreeLight = XmasTreeLight.Cast( ScriptedLightBase.CreateLight( XmasTreeLight, "0 0 0" ) );
			m_TreeLight.AttachOnMemoryPoint( this, "action" );
		}
	}
	
	override void EEDelete(EntityAI parent)
	{
		if ( !GetGame().IsMultiplayer() || GetGame().IsClient() )
		{
			if ( m_TreeLight )
				m_TreeLight.Destroy();
		}
	}
};
class ChristmasTree_Green extends ChristmasTree {};