/*
DISCLAIMER: may undergo some changes in the course of 1.14 experimental stage.
*/

class PPEManagerStatic
{
	static ref PPEManager m_Manager;
	
	static void CreateManagerStatic()
	{
		if (m_Manager)
		{
			Debug.Log("PPEManagerStatic | CreateManagerStatic - PPEManager already exists");
			return;
		}
		
		m_Manager = new PPEManager;
	}
	
	static PPEManager GetPPEManager()
	{
		return m_Manager;
	}
}

class PPEManager extends Managed
{
	const int CAMERA_ID = 0;
	
	protected bool 									m_ManagerInitialized;
	protected ref map<int, ref PPEClassBase> 		m_PPEClassMap; //contains sorted postprocess classes, IDs in 'PostProcessEffectType' // <MaterialID,<material_class>>
	protected ref map<int, ref array<int>> 			m_PPEMaterialUpdateQueueMap; //multiple levels of update queues, to allow for multiple dependent updates during same frame (greedy?)
	protected ref array<int> 						m_UpdatedMaterials;
	protected ref array<ref PPERequesterBase> 		m_ExistingPostprocessRequests; //which requests are active overall. Does not have to be updating ATM!
	protected ref array<ref PPERequesterBase> 		m_UpdatingRequests; //which requests are currently updating and processing
	
	void PPEManager()
	{
		m_ManagerInitialized = false;
		PPERequesterBank.Init();
	}
	
	void Init()
	{
		//DbgPrnt("PPEDebug | PPEManager | m_ManagerInitialized: " + m_ManagerInitialized);
		if (!m_ManagerInitialized)
		{
			m_PPEMaterialUpdateQueueMap = new map<int, ref array<int>>;
			m_UpdatedMaterials = new array<int>;
			m_ExistingPostprocessRequests = new array<ref PPERequesterBase>;
			m_UpdatingRequests = new array<ref PPERequesterBase>;
			InitPPEManagerClassMap();
			
			GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert(this.Update); //can be safely and easily 'disabled' here
			m_ManagerInitialized = true;
		}
	}
	
	//! Ordered by 'PostProcessEffectType' for easy access through the same enum; ID saved all the same
	void InitPPEManagerClassMap()
	{
		if (m_PPEClassMap)
		{
			delete m_PPEClassMap;
		}
		m_PPEClassMap = new map<int, ref PPEClassBase>;
		
		RegisterPPEClass(new PPENone()); //dummy
		RegisterPPEClass(new PPEUnderWater());
		RegisterPPEClass(new PPESSAO());
		RegisterPPEClass(new PPEDepthOfField());
		RegisterPPEClass(new PPEHBAO());
		RegisterPPEClass(new PPERotBlur());
		RegisterPPEClass(new PPEGodRays());
		RegisterPPEClass(new PPERain());
		RegisterPPEClass(new PPEFilmGrain());
		RegisterPPEClass(new PPERadialBlur());
		RegisterPPEClass(new PPEChromAber());
		RegisterPPEClass(new PPEWetDistort());
		RegisterPPEClass(new PPEDynamicBlur());
		RegisterPPEClass(new PPEColorGrading());
		RegisterPPEClass(new PPEColors());
		RegisterPPEClass(new PPEGlow());
		RegisterPPEClass(new PPESMAA());
		RegisterPPEClass(new PPEFXAA());
		RegisterPPEClass(new PPEMedian());
		RegisterPPEClass(new PPESunMask());
		RegisterPPEClass(new PPEGaussFilter());
		RegisterPPEClass(new PPEExposureNative());
		RegisterPPEClass(new PPEEyeAccomodationNative());
		RegisterPPEClass(new PPEDOF());
		RegisterPPEClass(new PPELightIntensityParamsNative());
	}
	
	//! Registeres material class and creates data structure within
	void RegisterPPEClass(PPEClassBase material_class)
	{
		m_PPEClassMap.Set(material_class.GetPostProcessEffectID(), material_class);
	}
	
	void SendMaterialValueData(PPERequestParamDataBase data)
	{
		//DbgPrnt("DataVerification | m_ColorTarget | SendMaterialValueData: " + PPERequestParamDataColor.Cast(data).m_ColorTarget[0] + "/" + PPERequestParamDataColor.Cast(data).m_ColorTarget[1] + "/" + PPERequestParamDataColor.Cast(data).m_ColorTarget[2] + "/" + PPERequestParamDataColor.Cast(data).m_ColorTarget[3]);
		PPEClassBase mat_class = m_PPEClassMap.Get(data.GetMaterialID());
		mat_class.InsertParamValueData(data);
		SetMaterialParamUpdating(data.GetMaterialID(),data.GetParameterID(),PPEConstants.DEPENDENCY_ORDER_BASE);
	}
	
	//! Queues material/parameter to update (once)
	void SetMaterialParamUpdating(int material_id, int parameter_id, int order)
	{
		if ( order > PPEConstants.DEPENDENCY_ORDER_HIGHEST )
		{
			//DbgPrnt("PPEDebug | PPEManager - SetMaterialParamUpdating | Order higher than max, ignoring! | mat/par/ord: " + material_id + "/" + parameter_id + "/" + order);
			return;
		}
		
		PPEClassBase mat_class = m_PPEClassMap.Get(material_id);
		
		//DbgPrnt("PPEDebug | PPEManager - SetMaterialParamUpdating | mat/par: " + material_id + "/" + parameter_id);
		//insert material into queue
		if ( !m_PPEMaterialUpdateQueueMap.Contains(order) )
			m_PPEMaterialUpdateQueueMap.Set(order,new array<int>);
		
		int found = m_PPEMaterialUpdateQueueMap.Get(order).Find(material_id);
		if ( found == -1 )
		{
			m_PPEMaterialUpdateQueueMap.Get(order).Insert(material_id);
		}
		
		mat_class.SetParameterUpdating(order,parameter_id);
	}
	
	//TODO - revise
	void RemoveMaterialUpdating(int material_id, int order = 0)
	{
		if ( m_PPEMaterialUpdateQueueMap.Contains(order) )
		{
			m_PPEMaterialUpdateQueueMap.Get(order).RemoveItem(material_id);
			
			if ( m_PPEMaterialUpdateQueueMap.Get(order).Count() == 0)
				m_PPEMaterialUpdateQueueMap.Remove(order);
		}
	}
	
	void ClearMaterialUpdating()
	{
		m_PPEMaterialUpdateQueueMap.Clear();
	}
	
	//TODO - partially unused
	void SetRequestActive(PPERequesterBase request, bool active)
	{
		int found = m_ExistingPostprocessRequests.Find(request);
		if ( active && found == -1 )
		{
			m_ExistingPostprocessRequests.Insert(request);
		}
		else if ( !active && found > -1 ) //should always be found in this case, redundant?
		{
			//RemoveActiveRequestFromMaterials(request);
			
			m_ExistingPostprocessRequests.Remove(found);
		}
	}
	
	void SetRequestUpdating(PPERequesterBase request, bool active)
	{
		if (!m_UpdatingRequests)
		{
			Debug.Log("PPEManager | SetRequestUpdating | !m_UpdatingRequests");
			return;
		}
		
		int idx = m_UpdatingRequests.Find(request);
		
		if ( active && idx == -1 )
		{
			m_UpdatingRequests.Insert(request);
		}
		else if ( !active && idx > -1 )
		{
			m_UpdatingRequests.Remove(idx);
		}
	}
	
	//TODO - do we need this?
	bool GetExistingRequester(typename req, out PPERequesterBase ret)
	{
		int idx = m_ExistingPostprocessRequests.Find(PPERequesterBank.GetRequester(req));
		if (idx > -1)
		{
			ret = m_ExistingPostprocessRequests.Get(idx);
			return true;
		}
		return false;
	}
	
	//unused?
	//! Remove requester data from all relevant materials
	private void RemoveActiveRequestFromMaterials(PPERequesterBase req)
	{
		int count = req.GetActiveRequestStructure().Count();
		int mat_id;
		for (int i = 0; i < count; i++)
		{
			mat_id = req.GetActiveRequestStructure().GetKey(i);
			m_PPEClassMap.Get(mat_id).RemoveRequest(req.GetRequesterIDX());
		}
	}
	
	//TODO - old stuff, rework
	private void RequestsCleanup()
	{
	}
	
	void InsertUpdatedMaterial(int mat_id)
	{
		if ( m_UpdatedMaterials.Find(mat_id) == -1 )
			m_UpdatedMaterials.Insert(mat_id);
	}
	
	//---------//
	//PROCESSING
	//---------//
	private void ProcessRequesterUpdates(float timeslice)
	{
		PPERequesterBase req;
		for (int i = 0; i < m_UpdatingRequests.Count(); i++)
		{
			//DbgPrnt("PPEDebug | ProcessRequesterUpdates | m_UpdatingRequests[i]: " + m_UpdatingRequests[i]);
			req = m_UpdatingRequests.Get(i);
			if (req)
				req.OnUpdate(timeslice);
		}
	}
	
	private void ProcessMaterialUpdates(float timeslice)
	{
		for (int i = 0; i < m_PPEMaterialUpdateQueueMap.Count(); i++) //orders (levels?)
		{
			//DbgPrnt("PPEDebug | ProcessMaterialUpdates | GetKey " + i + ": " + m_PPEMaterialUpdateQueueMap.GetKey(i));
			//DbgPrnt("PPEDebug | ProcessMaterialUpdates | GetElement - count " + i + ": " + m_PPEMaterialUpdateQueueMap.GetElement(i).Count());
			
			for (int j = 0; j < m_PPEMaterialUpdateQueueMap.GetElement(i).Count(); j++)
			{
				m_PPEClassMap.Get(m_PPEMaterialUpdateQueueMap.GetElement(i).Get(j)).OnUpdate(timeslice,i);
			}
		}
	}
	
	private void ProcessApplyValueChanges()
	{
		int material_id;
		for (int i = 0; i < m_UpdatedMaterials.Count(); i++)
		{
			material_id = m_UpdatedMaterials.Get(i);
			m_PPEClassMap.Get(material_id).ApplyValueChanges();
		}
		
		m_UpdatedMaterials.Clear();
		ClearMaterialUpdating();
	}
	
	void Update(float timeslice)
	{
		if (!m_ManagerInitialized)
			return;
		
		ProcessRequesterUpdates(timeslice);
		ProcessMaterialUpdates(timeslice);
		ProcessApplyValueChanges();
		//RequestsCleanup();
	}
	
	Param GetPostProcessDefaultValues(int material, int parameter)
	{
		return m_PPEClassMap.Get(material).GetParameterCommandData(parameter).GetDefaultValues();
	}
	
	Param GetPostProcessCurrentValues(int material, int parameter)
	{
		return m_PPEClassMap.Get(material).GetParameterCommandData(parameter).GetCurrentValues();
	}
	
	//will be used very rarely, mostly set in C++
	//TODO - option change handling (expose from C++)
	void ChangePPEMaterial(PostProcessPrioritiesCamera priority, PostProcessEffectType type, string path, bool scriptside_only)
	{
		if (m_PPEClassMap.Contains(type))
		{
			typename name = m_PPEClassMap.Get(type).Type();
			PPEClassBase postprocess_capsule = PPEClassBase.Cast(name.Spawn());
			postprocess_capsule.ChangeMaterialPathUsed(path);
			
			if (postprocess_capsule.GetMaterial() == 0x0)
			{
				Debug.Log("PPEManager | Invalid material path " + path + " used for " + name );
				return;
			}
			
			//m_PPEClassMap.Remove(type);
			m_PPEClassMap.Set(type,postprocess_capsule);
		}
		
		//can be sent script-side only to adapt to c++ options changes
		if (!scriptside_only)
			SetCameraPostProcessEffect(CAMERA_ID,priority,type,path);
	}
	
	void StopAllEffects(int mask = 0)
	{
		if (m_ExistingPostprocessRequests)
		{		
			foreach (PPERequesterBase requester : m_ExistingPostprocessRequests)
			{
				if (requester.GetCategoryMask() & mask)
				{
					requester.Stop();
				}
			}
		}
	}
	
	void DbgPrnt(string text)
	{
		//Debug.Log(""+text);
	}
};
