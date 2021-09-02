// Base class for "Effect triggers"
// Registers in TriggerEffectManager and handles parameter setting through cfgEffectArea.json file
class EffectTrigger : CylinderTrigger
{
	int m_AroundPartId; // The main particles spawned around player when in trigger
	int m_TinyPartId; // The smaller particles spawned around player when in trigger
	int m_PPERequester; // The Post Processing used when player is in trigger
	float m_DeltaTime;
	float m_TimeAccuStay =;
	TriggerEffectManager m_Manager;
	
	void EffectTrigger()
	{
		RegisterNetSyncVariableInt("m_AroundPartId");
		RegisterNetSyncVariableInt("m_TinyPartId");
		RegisterNetSyncVariableInt("m_PPERequester");
		
		m_Manager = TriggerEffectManager.GetInstance();
		m_Manager.SetTriggerType( this );
	}
	
	// ----------------------------------------------
	// 				CUSTOM EVENTS
	// ----------------------------------------------
	
	void SetLocalEffects( int aroundPartId, int tinyPartId, int ppeRequesterIdx )
	{
		m_AroundPartId = aroundPartId;
		m_TinyPartId = tinyPartId;
		m_PPERequester = ppeRequesterIdx;
		
		SetSynchDirty();
	}
	
	// ----------------------------------------------
	// 				TRIGGER EVENTS
	// ----------------------------------------------
	
	override void OnEnterServerEvent( TriggerInsider insider )
	{
		super.OnEnterServerEvent( insider );
	}
	
	override void OnEnterClientEvent( TriggerInsider insider )
	{
		super.OnEnterClientEvent( insider );
	}
	
	override void OnLeaveServerEvent( TriggerInsider insider )
	{
		super.OnLeaveServerEvent( insider );
	}
	
	override void OnLeaveClientEvent( TriggerInsider insider )
	{
		super.OnLeaveClientEvent( insider );
	}
	
	// We remove from trigger update DEAD or RESISTANT entities to limit the amount of entities to update
	override bool ShouldRemoveInsider( TriggerInsider insider )
	{
		return !insider.GetObject().IsAlive();
	}
	
	// Used to apply the desired effect to all entities present in one trigger of the specified type
	// NOTE : This is really not optimal, if you want to add new trigger types, you will have to test for them...
	static void TriggerEffect( EntityAI insider, typename triggerType ) {}
}
