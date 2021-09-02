class GameplayEffectWidgets_base extends Managed
{
	bool IsAnyEffectRunning(){}
	bool AreEffectsSuspended(){}
	void AddActiveEffects(array<int> effects){}
	void RemoveActiveEffects(array<int> effects){}
	void StopAllEffects(){}
	void AddSuspendRequest(int request_id){}
	void RemoveSuspendRequest(int request_id){}
	void ClearSuspendRequests(){}
	int GetSuspendRequestCount(){}
	void UpdateWidgets(int type = -1, float timeSlice = 0, Param p = null, int handle = -1){}
	void Update(float timeSlice){}
	void SetBreathTime(float value){}
	void SetBreathIntensityStamina(float stamina_cap, float stamina_current){}
}