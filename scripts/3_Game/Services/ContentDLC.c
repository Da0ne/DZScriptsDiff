
// Binded values from engine
enum EDLCId
{
	DLC_UNKNOWN,
	DLC_BLISS,
};

//! ContentDLC is for query installed DLC (only as entitlement keys, not content)
class ContentDLC
{
	/*!
		For getting current entitlements on device (console). Could be called 
		after any entitlement change or before accessing locked content.
		\param dlcIdList through this list is returned all installed DLC with valid license for current user
		\return count of installed and valid DLCs.
	*/
	proto native int GetEntitlements(out TIntArray dlcIdList);
	
	/*!
		\param dlcId ID of DLC
		\return current state of availability of unlocking' dlc. Only DLC with valid license return true.
	*/
	proto native bool IsDLCInstalled(EDLCId dlcId);
	
	/*!
		Called from engine for notifing script after any change in additional 
		content (installed, lost license). There should be check for current state of entitlements.
		On Xbox should be problem with event lost license! (system bug)
		\param dlcId ID of DLC that was changed. Could be empty string if console does not report concrete DLC ID.
	*/
	void OnChange(EDLCId dlcId)
	{
		Print("ContentDLC: OnChange, id: " + dlcId);
		
		
		
		
		//-----only-for-test--change-it----------
		TIntArray ids = new TIntArray;
		int count = GetEntitlements(ids);
		Print("ContentDLC: User accessed " + count + " entitlements");
		
		for (int i = 0; i < ids.Count(); i++)
		{
			Print("    ->" + ids[i]);
		}
		//---------------------------------------
	}
};
