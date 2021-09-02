/*
DISCLAIMER: may undergo some changes in the course of 1.14 experimental stage.
*/

/**
	\brief Requester bank contains all registered type instances as singletons. Creating new instances outside of bank might be a bad idea (and a way to memory leakage)
*/
class PPERequesterBank
{
	private static ref map<int,ref PPERequesterBase> m_Instances;
	private static bool m_Initialized = false;
	
	static int m_lastID = -1;
	
	static const int INVALID					= -1;
	static const int REQ_INVENTORYBLUR			= RegisterRequester(PPERequester_InventoryBlur);
	static const int REQ_CONTROLLERDISCONNECT	= RegisterRequester(PPERequester_ControllerDisconnectBlur);
	static const int REQ_GLASSESSPORTBLACK		= RegisterRequester(PPERequester_GlassesSportBlack);
	static const int REQ_GLASSESSPORTBLUE		= RegisterRequester(PPERequester_GlassesSportBlue);
	static const int REQ_GLASSESSPORTGREEN		= RegisterRequester(PPERequester_GlassesSportGreen);
	static const int REQ_GLASSESSPORTORANGE		= RegisterRequester(PPERequester_GlassesSportOrange);
	static const int REQ_GLASSESAVIATOR			= RegisterRequester(PPERequester_GlassesAviator);
	static const int REQ_GLASSESDESIGNER		= RegisterRequester(PPERequester_GlassesDesignerBlack);
	static const int REQ_GLASSESTACTICAL		= RegisterRequester(PPERequester_TacticalGoggles);
	static const int REQ_MOTOHELMETBLACK		= RegisterRequester(PPERequester_MotoHelmetBlack);
	
	static const int REQ_GLASSESWELDING			= RegisterRequester(PPERequester_WeldingMask);
	static const int REQ_CAMERANV				= RegisterRequester(PPERequester_CameraNV);
	static const int REQ_CAMERAADS				= RegisterRequester(PPERequester_CameraADS);
	static const int REQ_BLOODLOSS				= RegisterRequester(PPERequester_BloodLoss);
	static const int REQ_DEATHEFFECTS			= RegisterRequester(PPERequester_DeathDarkening);
	static const int REQ_UNCONEFFECTS			= RegisterRequester(PPERequester_UnconEffects);
	static const int REQ_TUNELVISSION			= RegisterRequester(PPERequester_TunnelVisionEffects);
	static const int REQ_BURLAPSACK				= RegisterRequester(PPERequester_BurlapSackEffects);
	static const int REQ_INTROCHROMABB			= RegisterRequester(PPERequester_IntroChromAbb);
	static const int REQ_FEVEREFFECTS			= RegisterRequester(PPERequester_FeverEffects);
	static const int REQ_FLASHBANGEFFECTS		= RegisterRequester(PPERequester_FlashbangEffects);
	static const int REQ_SHOCKHITEFFECTS		= RegisterRequester(PPERequester_ShockHitReaction);
	static const int REQ_HEALTHHITEFFECTS		= RegisterRequester(PPERequester_HealthHitReaction);
	static const int REQ_MENUEFFECTS			= RegisterRequester(PPERequester_MenuEffects);
	static const int REQ_CONTROLLERBLUR			= RegisterRequester(PPERequester_ControlsBlur);
	static const int REQ_SERVERBROWSEREFFECTS	= RegisterRequester(PPERequester_ServerBrowserBlur);
	static const int REQ_TUTORIALEFFECTS		= RegisterRequester(PPERequester_TutorialMenu);
	static const int REQ_CONTAMINATEDAREA		= RegisterRequester(PPERequester_ContaminatedAreaTint);
	static const int REQ_PAINBLUR				= RegisterRequester(PPERequester_PainBlur);
	
	private static ref PPERequesterRegistrations 	m_Registrations; //more registrations to be placed here
	
	static void Init()
	{
		m_Registrations = new PPERequesterRegistrations;
		
		if (!m_Instances)
			m_Instances = new map<int,ref PPERequesterBase>;
		
		m_Initialized = true;
	}
	
	/**
	\brief Returns an instance (singleton) of a requester based on typename.
	\param type \p typename Typename of the requester.
	\return \p PPERequesterBase Requester singleton.
	\note Returns only already registered instances, does not register anything.
	*/
	static PPERequesterBase GetRequester(typename type)
	{
		PPERequesterBase temp;
		PPERequesterBase ret;
		for (int i = 0; i < m_Instances.Count(); i++)
		{
			temp = m_Instances.GetElement(i);
			if (temp.Type() == type)
			{
				Class.CastTo(ret,temp);
				break;
			}
		}
		
		if (!ret)
		{
			Debug.Log("" + type.ToString() + " not found in bank! Register first in 'RegisterRequester' method.");
		}
		return ret;
	}
	
	/**
	\brief Returns an instance (singleton) of a requester based on index.
	\param index \p int Index of the registered requester.
	\return \p PPERequesterBase Requester singleton.
	\note Returns only already registered instances, does not register anything.
	*/
	static PPERequesterBase GetRequester(int index)
	{
		PPERequesterBase ret = m_Instances.Get(index);
		
		if (!ret)
		{
			Debug.Log("Requester idx " + index + " not found in bank! Register first in 'RegisterRequester' method.");
		}
		return ret;
	}
	
	/**
	\brief Registers new requester type.
	\note Performed only on game start. New requesters can be registered in 'PPERequesterRegistrations'
	*/
	static int RegisterRequester(typename type)
	{
		//Print("RegisterRequester: " + type.ToString());
		if (!m_Instances)
			m_Instances = new map<int,ref PPERequesterBase>;
		
		if ( GetRequester(type) != null )
		{
			Error("Trying to register an already existing requester type: " + type); //TODO - Debug.Log later
			return -1;
		}
		
		m_lastID++;
		
		PPERequesterBase req;
		req = PPERequesterBase.Cast(type.Spawn());
		req.SetRequesterIDX(m_lastID);
		m_Instances.Set(m_lastID,req);
		
		return m_lastID;
	}
	
	//! Verifies the instance - can't be registered during 
	static bool VerifyRequester(PPERequesterBase req)
	{
		if (!m_Initialized) //initial registrations are fair game
		{
			return true;
		}
		
		PPERequesterBase temp;
		for (int i = 0; i < m_Instances.Count(); i++)
		{
			temp = m_Instances.GetElement(i);
			if (temp == req)
			{
				return true;
			}
		}
		
		Error("Requester instance " + req + " not valid! Please use registered instances from PPERequesterBank.");
		return false;
	}
	
	/*static void DumpInstances()
	{
		Print("DumpInstances");
		for (int i = 0; i < m_Instances.Count(); i++)
		{
			Print(m_Instances.GetKey(i));
			Print(m_Instances.GetElement(i));
		}
	}*/
}

//! Mod this, if you wish to register custom requesters
class PPERequesterRegistrations extends Managed
{
	void PPERequesterRegistrations()
	{
		//PPERequesterBank.RegisterRequester(TestReq);
	}
}

/*modded class PPERequesterRegistrations extends Managed
{
	void PPERequesterRegistrations()
	{
		PPERequesterBank.RegisterRequester(TestReq2);
	}
}*/