/*
DISCLAIMER: may undergo some changes in the course of 1.14 experimental stage.
*/

// Used for Effect based triggers ( such as Contaminated areas )
// Allows us to unify these triggers and treat Enter / Exit events as if all triggers were one
class TriggerEffectManager
{
	protected EffectTrigger m_Trigger;
	
	static ref TriggerEffectManager m_TriggerInstance;
	
	// Player map will only be handled client side as it is most relevant
	// AI agent map will only be handled server side as it is most relevant ( ai map does not hash entity as it needs the specific references ) 
	ref map<string, ref map<PlayerBase, int>> m_PlayerInsiderMap; // Used to keep track of which players are in which effects
	ref map<int, ref map<DayZCreatureAI, int>> m_AIInsiderMap; // Same as above but specifically for AI agents in order to shorten player search times
	
	ref array<typename> m_TriggerTypeList; // Contains a list of all the currently active trigger types
	
	// Each trigger type we want to pool in this fashion will have a ref to this class
	void TriggerEffectManager( ContaminatedTrigger effectTrigger ) { }
	
	
	static void DestroyInstance()
	{
		m_TriggerInstance = null;
	}
	
	// This is a SINGLETON so, if we can't get the instance, we create it
	static TriggerEffectManager GetInstance()
	{
		if ( m_TriggerInstance )
			return m_TriggerInstance;
		
		m_TriggerInstance = new TriggerEffectManager( null );
		return m_TriggerInstance;
	}
	
	void SetTriggerType( EffectTrigger effectTrigger )
	{
		m_Trigger = effectTrigger;
		
		// We must initialize this map on both server and client
		SetupPlayerMapping();
		
		// We must ONLY initialize this map on server
		if ( GetGame().IsServer() || !GetGame().IsMultiplayer() )
			SetupAIMapping();
		
		// We create the array if it doesn't already exist
		if ( !m_TriggerTypeList )
			m_TriggerTypeList = new ref array<typename>;
		
		// If we find the trigger type in the array, no need to proceed
		if ( m_TriggerTypeList.Find( m_Trigger.Type() ) != -1 )
			return;
		
		//Debug.Log("We insert " + m_Trigger.Type().ToString() );
		// We insert the trigger type to the array, we have one instance of it live in the world
		m_TriggerTypeList.Insert( m_Trigger.Type() );
	}
	
	// Setup of the map used to track which players are in which trigger types or not
	void SetupPlayerMapping()
	{
		if ( !m_PlayerInsiderMap )
			m_PlayerInsiderMap = new map<string, ref map<PlayerBase, int>>;
		
		string triggerType = m_Trigger.GetType();
		
		if ( !m_PlayerInsiderMap.Contains( triggerType ) )
		{
			map<PlayerBase, int> playerMap = new map<PlayerBase, int>;
			m_PlayerInsiderMap.Insert( triggerType, playerMap );
			
			//Debug.Log( "We have : " + m_PlayerInsiderMap.Count() + " triggers");
		}
	}
	
	// Setup of the map used to track which ai agents are in which trigger types or not
	void SetupAIMapping()
	{
		if ( !m_AIInsiderMap )
			m_AIInsiderMap = new map<int, ref map<DayZCreatureAI, int>>;
		
		string triggerType = m_Trigger.GetType();
		int hashedType = triggerType.Hash();
		
		if ( !m_AIInsiderMap.Contains( hashedType ) )
		{
			map<DayZCreatureAI, int> aiMap = new map<DayZCreatureAI, int>;
			m_AIInsiderMap.Insert( hashedType, aiMap );
			
			//Debug.Log( "We have : " + m_AIInsiderMap.Count() + " triggers");
		}
	}
	
	// When a player enters a trigger using this class, we add player to map or increase it's trigger count
	void OnPlayerEnter( notnull PlayerBase player, notnull EffectTrigger trigger )
	{
		string trigger_type = trigger.GetType();
		
		map<PlayerBase, int> playerMap;
		if ( m_PlayerInsiderMap.Find( trigger_type, playerMap) )
		{
			if ( playerMap )
			{ 
				if ( playerMap.Contains( player ) )
				{
					int newTriggerCount = playerMap.Get( player ) + 1;
					playerMap.Set( player, newTriggerCount );
					//Debug.Log("We increase the amount of triggers a player is in");
				}
				else
				{
					// First time this player enters this type of trigger, we set the trigger count to 1
					playerMap.Insert( player, 1 );
					//Debug.Log("We insert a player");
				}
				
				//m_PlayerInsiderMap.Set( trigger_type, playerMap );
			}
			else
			{
				// We didn't have the map yet, we create it and register this player in it
				playerMap = new map<PlayerBase, int>;
				playerMap.Insert( player, 1 );
				m_PlayerInsiderMap.Insert( trigger_type, playerMap );
			}
		}
	}
	
	// Same as OnPlayerEnter, but we decrease trigger count on each trigger leave and remove player from map when count is 0
	void OnPlayerExit( notnull PlayerBase player, notnull EffectTrigger trigger )
	{
		string trigger_type = trigger.GetType();
		
		map<PlayerBase, int> playerMap;
		if ( m_PlayerInsiderMap.Find( trigger_type, playerMap ) && playerMap )
		{
			if ( playerMap.Contains( player ))
			{
				if( playerMap.Get( player ) > 1 )
				{
					//Debug.Log("We reduced the amount of triggers the player is in");
					playerMap.Set( player, playerMap.Get( player ) - 1 );
				}
				else
				{
					// The player left the last trigger of this type they were in
					playerMap.Remove( player );
					//Debug.Log("We removed this player from this trigger type");
				}
			}
			//m_PlayerInsiderMap.Set( trigger_type, playerMap );

		}
	}
	
	bool IsPlayerInTriggerType( notnull PlayerBase player, notnull EffectTrigger trigger )
	{
		map<PlayerBase, int> playerMap;
		if ( m_PlayerInsiderMap.Find( trigger.GetType(), playerMap ) )
		{
			return playerMap != null && playerMap.Contains( player );
		}
		
		// If we do not find the specified player at all, he just left the trigger type
		return false;
	}
}