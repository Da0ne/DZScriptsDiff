/*
DISCLAIMER: may undergo some changes in the course of 1.14 experimental stage.
*/

//!data for one material parameter, requester side
class PPERequestParamDataBase
{
	ref PPERequesterBase m_Requester;
	protected bool m_UpdatingDataValues; //new values are being sent periodically
	protected bool m_IsActive; //hasn't been stopped yet (does not have to update values)
	protected bool m_SettingDefaultValues; //NOTE - used
	
	protected int m_MaterialID;
	protected int m_ParameterID;
	protected int m_VariableType; //PPEConstants.VAR_TYPE_INT etc.
	protected int m_RequesterIDX;
	protected int m_InteractionMask;
	protected int m_Priority; //de-facto layer
	
	//TODO
	//MAT_DECLARE_TEXTURE - string?
	//DECLARE_RESOURCE_NAME - string + data?
	
	void PPERequestParamDataBase(int requester_idx, int mat_id, int param_id, int data_type = 0, int priority = 0, int mask = PPOperators.SET)
	{
		m_RequesterIDX = requester_idx;
		m_MaterialID = mat_id;
		m_ParameterID = param_id;
		m_VariableType = data_type;
		m_Priority = priority;
		m_InteractionMask = mask;
		m_UpdatingDataValues = false;
		m_SettingDefaultValues = false;
		m_IsActive = false;
		InitValues();
	}
	
	void InitValues()
	{
	}
	
	void SetUpdatingDataValues(bool state)
	{
		m_UpdatingDataValues = state;
	}
	
	bool GetUpdatingDataValues()
	{
		return m_UpdatingDataValues;
	}
	
	void SetDataActive(bool state)
	{
		m_IsActive = state;
	}
	
	bool IsDataActive()
	{
		return m_IsActive;
	}
	
	void SetSettingDefaultValues(bool state)
	{
		m_SettingDefaultValues = state;
	}
	
	bool IsSettingDefaultValues()
	{
		return m_SettingDefaultValues;
	}
	
	int GetRequesterIDX()
	{
		return m_RequesterIDX;
	}
	
	int GetMaterialID()
	{
		return m_MaterialID;
	}
	
	int GetParameterID()
	{
		return m_ParameterID;
	}
	
	int GetVarType()
	{
		return m_VariableType;
	}
	
	void SetInteractionMask(int mask)
	{
		m_InteractionMask = mask;
	}
	
	int GetInteractionMask()
	{
		return m_InteractionMask;
	}
	
	void SetPriorityLayer(int priority)
	{
		m_Priority = priority;
	}
	
	int GetPriorityLayer()
	{
		return m_Priority;
	}
}

class PPERequestParamDataBool extends PPERequestParamDataBase
{
	bool m_BoolTarget;
	bool m_BoolCurrent;
	bool m_BoolLast;
	
	void PPERequestParamDataBool(int requester_idx, int mat_id, int param_id, int data_type = 0, int priority = 0, int mask = PPOperators.SET, bool relative = false )
	{
		m_BoolCurrent = 0;
		m_BoolLast = 0;
		m_VariableType = PPEConstants.VAR_TYPE_BOOL;
	}
}

class PPERequestParamDataInt extends PPERequestParamDataBase
{
	int m_IntTarget;
	int m_IntCurrent;
	int m_IntLast;
	
	void PPERequestParamDataInt(int requester_idx, int mat_id, int param_id, int data_type = 0, int priority = 0, int mask = PPOperators.SET, bool relative = false )
	{
		m_IntCurrent = -1;
		m_IntLast = -1;
		m_VariableType = PPEConstants.VAR_TYPE_INT;
	}
}

class PPERequestParamDataFloat extends PPERequestParamDataBase
{
	float m_FloatTarget;
	float m_FloatCurrent;
	float m_FloatLast;
	//?
	float m_FloatFormerTarget;//former target
	float m_FloatStart;//actual start value
	
	void PPERequestParamDataFloat(int requester_idx, int mat_id, int param_id, int data_type = 0, int priority = 0, int mask = PPOperators.SET, bool relative = false )
	{
		m_VariableType = PPEConstants.VAR_TYPE_FLOAT;
	}
	
	override void InitValues()
	{
		m_FloatCurrent = -1;
		m_FloatLast = -1;
		m_FloatTarget = 0.0;
		m_FloatCurrent = 0.0;
		m_FloatLast = 0.0;
		m_FloatFormerTarget = 0.0;
		m_FloatStart = 0.0;
	}
}

class PPERequestParamDataColor extends PPERequestParamDataBase
{
	ref array<float>  m_ColorTarget;
	ref array<float>  m_ColorCurrent;
	ref array<float>  m_ColorLast;
	
	ref array<float>  m_ColorFormerTarget;//former target
	ref array<float>  m_ColorStart;//actual start values
	
	void PPERequestParamDataColor(int requester_idx, int mat_id, int param_id, int data_type = 0, int priority = 0, int mask = PPOperators.SET, bool relative = false )
	{
		m_VariableType = PPEConstants.VAR_TYPE_COLOR;
	}
	
	override void InitValues()
	{
		m_ColorTarget = {0.0,0.0,0.0,0.0};
		m_ColorCurrent = {0,0.0,0.0,0.0};
		m_ColorLast = {0.0,0.0,0.0,0.0};
		m_ColorFormerTarget = {0.0,0.0,0.0,0.0};
		m_ColorStart = {0.0,0.0,0.0,0.0};
	}
}

//TODO
class PPERequestParamDataVector extends PPERequestParamDataBase
{
	vector m_VectorTarget;
	vector m_VectorCurrent;
	vector m_VectorLast;
}
