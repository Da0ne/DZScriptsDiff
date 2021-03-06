typedef Param3<string, bool, bool> PresetParams;
typedef Param5<bool, string, int, string, int> ConfigParams; // param1 - isCollapsed, param2 - string name, param3 - num of childs, param4 - path, param5 - deep

class ScriptConsole extends UIScriptedMenu
{
	const string  DEFAULT_POS_XYZ = "<1,2,3>";
	vector m_MapPos;
	bool m_PlayerPosRefreshBlocked;
	const string CONST_DEFAULT_PRESET_PREFIX = "[Default]";
	static float DEBUG_MAP_ZOOM = 1;
	static int ITEMS_SELECTED_ROW = -1;
	static int PRESETS_SELECTED_ROW = 0;
	static int ITEMS_IN_PRESET_SELECTED_ROW;
	static int m_ObjectsScope = 2;
	static bool SHOW_OTHERS = 0;
	static bool CLEAR_IVN;
	static float DRAW_DISTANCE = 1000;
	static ref array<ref Shape> m_DebugShapes = new array<ref Shape>;
	
	
	ref Timer m_LateInit = new Timer();
	bool m_ScriptServer;
	bool m_UpdateMap;
	void ScriptConsole()
	{
		m_ModuleLocalEnscriptHistory 	= PluginLocalEnscriptHistory.Cast( GetPlugin(PluginLocalEnscriptHistory) );
		m_ModuleLocalEnscriptHistoryServer 	= PluginLocalEnscriptHistoryServer.Cast( GetPlugin(PluginLocalEnscriptHistoryServer) );
		
	}

	void ~ScriptConsole()
	{
		
		DEBUG_MAP_ZOOM = m_DebugMapWidget.GetScale();
		
		ITEMS_SELECTED_ROW = m_ObjectsTextListbox.GetSelectedRow();
		PRESETS_SELECTED_ROW = m_PresetsTextListbox.GetSelectedRow();
		ITEMS_IN_PRESET_SELECTED_ROW = m_PresetItemsTextListbox.GetSelectedRow();
				
		m_PresetsTextListbox.ClearItems();
		
		PluginRemotePlayerDebugClient plugin_remote_client = PluginRemotePlayerDebugClient.Cast( GetPlugin(PluginRemotePlayerDebugClient) );
		if (SHOW_OTHERS && plugin_remote_client)
			plugin_remote_client.RequestPlayerInfo( PlayerBase.Cast(GetGame().GetPlayer()), 0 );
	}
	
#ifdef X1_TODO_TEMP_GUI
	bool UseMouse()
	{
		return true;
	}
#endif
	
	override Widget Init()
	{
		int i;
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("gui/layouts/script_console/script_console.layout");

		m_DebugMapWidget = MapWidget.Cast( layoutRoot.FindAnyWidget("MapWidget") );
		
		m_Tabs[TAB_ITEMS] = layoutRoot.FindAnyWidget("ItemsPanel");
		m_Tabs[TAB_CONFIGS] = layoutRoot.FindAnyWidget("ConfigsPanel");
		m_Tabs[TAB_ENSCRIPT] = layoutRoot.FindAnyWidget("EnScriptPanel");
		m_Tabs[TAB_ENSCRIPT_SERVER] = layoutRoot.FindAnyWidget("EnScriptPanel");
		m_Tabs[TABS_GENERAL] = layoutRoot.FindAnyWidget("GeneralPanel");
		m_Tabs[TABS_OUTPUT] = layoutRoot.FindAnyWidget("OutputPanel");

		m_Tab_buttons[TAB_ITEMS] = ButtonWidget.Cast( layoutRoot.FindAnyWidget("ItemsButtonWidget") );
		m_Tab_buttons[TAB_CONFIGS] = ButtonWidget.Cast( layoutRoot.FindAnyWidget("ConfigsButtonWidget") );
		m_Tab_buttons[TAB_ENSCRIPT] = ButtonWidget.Cast( layoutRoot.FindAnyWidget("EnScriptButtonWidget") );
		m_Tab_buttons[TAB_ENSCRIPT_SERVER] = ButtonWidget.Cast( layoutRoot.FindAnyWidget("EnScriptButtonWidgetServer") );
		m_Tab_buttons[TABS_GENERAL] = ButtonWidget.Cast( layoutRoot.FindAnyWidget("GeneralButtonWidget") );
		m_Tab_buttons[TABS_OUTPUT] = ButtonWidget.Cast( layoutRoot.FindAnyWidget("OutputButtonWidget") );

		m_ClientLogListbox = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("TextListbox") );
		m_ClientLogClearButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("ButtonClear") );
		m_ClientLogScrollCheckbox = CheckBoxWidget.Cast( layoutRoot.FindAnyWidget("CheckBoxAutoScroll") );

		m_DiagToggleTextListbox = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("DiagToggle") );
		m_DiagToggleButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("DiagToggleButton") );

		m_DiagDrawmodeTextListbox = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("DiagDrawmode") );
		m_DiagDrawButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("DiagDrawButton") );
		m_DrawInWorld = ButtonWidget.Cast( layoutRoot.FindAnyWidget("ButtonDrawItem") );
		m_DrawInWorldClear = ButtonWidget.Cast( layoutRoot.FindAnyWidget("ButtonDrawItemClear") );

		m_PositionsListbox	= TextListboxWidget.Cast( layoutRoot.FindAnyWidget("PositionsList") );
		m_TeleportButton	= ButtonWidget.Cast( layoutRoot.FindAnyWidget("ButtonTeleport") );
		m_ButtonCopyPos		= ButtonWidget.Cast( layoutRoot.FindAnyWidget("Button_CopyPos") );
		m_TeleportX			= EditBoxWidget.Cast( layoutRoot.FindAnyWidget("TeleportX") );
		m_TeleportY			= EditBoxWidget.Cast( layoutRoot.FindAnyWidget("TeleportY") );
		m_TeleportXYZ		= EditBoxWidget.Cast( layoutRoot.FindAnyWidget("TeleportXYZ") );
		m_PlayerCurPos		= TextWidget.Cast( layoutRoot.FindAnyWidget("PlayerPosLabel") );
		m_MouseCurPos		= TextWidget.Cast( layoutRoot.FindAnyWidget("MousePosLabel") );
		m_LogsEnabled		= CheckBoxWidget.Cast( layoutRoot.FindAnyWidget("cbx_LogsEnabled") );
		m_HudDCharStats		= CheckBoxWidget.Cast( layoutRoot.FindAnyWidget("cbx_CharacterStats") );
		m_HudDCharLevels	= CheckBoxWidget.Cast( layoutRoot.FindAnyWidget("cbx_CharacterLevels") );
		m_HudDCharStomach	= CheckBoxWidget.Cast( layoutRoot.FindAnyWidget("cbx_CharacterStomach") );
		m_HudDCharModifiers	= CheckBoxWidget.Cast( layoutRoot.FindAnyWidget("cbx_CharacterModifiers") );
		m_HudDCharAgents	= CheckBoxWidget.Cast( layoutRoot.FindAnyWidget("cbx_CharacterAgents") );
		m_HudDCharDebug		= CheckBoxWidget.Cast( layoutRoot.FindAnyWidget("cbx_CharacterDebug") );
		m_HudDFreeCamCross	= CheckBoxWidget.Cast( layoutRoot.FindAnyWidget("cbx_FreeCamCross") );
		m_HudDVersion		= CheckBoxWidget.Cast( layoutRoot.FindAnyWidget("cbx_Version") );
		
		m_ShowProtected		= CheckBoxWidget.Cast( layoutRoot.FindAnyWidget("ShowProtectedCheckbox") );
		m_ShowOthers		= CheckBoxWidget.Cast( layoutRoot.FindAnyWidget("ShowOthersCheckbox") );
		
		m_ClearInventory		= CheckBoxWidget.Cast( layoutRoot.FindAnyWidget("ForceClearCheckbox") );


		
		m_DrawDistanceWidget	= EditBoxWidget.Cast( layoutRoot.FindAnyWidget("DrawItemDistance") );
		
		m_EnfScriptEdit	= MultilineEditBoxWidget.Cast( layoutRoot.FindAnyWidget("MultilineEditBoxWidget0") );
		m_EnfScriptRun	= ButtonWidget.Cast( layoutRoot.FindAnyWidget("RunButton") );
		
		m_ObjectFilter = EditBoxWidget.Cast( layoutRoot.FindAnyWidget("ObjectFilter") );
		m_SpawnDistanceEditBox = EditBoxWidget.Cast( layoutRoot.FindAnyWidget("SpawnDistance") );
		m_SelectedObjectText = TextWidget.Cast( layoutRoot.FindAnyWidget("SelectedObject") );
		m_ObjectsTextListbox = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("ObjectsList") );
		m_PresetsTextListbox = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("PresetList") );
		m_PresetItemsTextListbox = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("PresetItemsList") );
		m_PresetNewButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("NewPresetButton") );
		m_PresetDeleteButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("DeletePresetButton") );
		m_PresetRenameButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("RenamePresetButton") );
		m_PresetSetDefaultButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("SetDefaultPresetButton") );
		m_PresetAddItemtButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("AddToPresetButton") );
		m_ItemNameCopy = ButtonWidget.Cast( layoutRoot.FindAnyWidget("ItemNameCopy") );
		m_PresetRemoveItemButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("RemoveFromPresetButton") );
		m_ItemMoveUpButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("ItemMoveUpButton") );
		m_ItemMoveDownButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("ItemMoveDownButton") );

		m_QuantityEditBox = EditBoxWidget.Cast( layoutRoot.FindAnyWidget("QuantityValue") );
		m_DamageEditBox = EditBoxWidget.Cast( layoutRoot.FindAnyWidget("DamageValue") );
		m_ItemDamageLabel = TextWidget.Cast( layoutRoot.FindAnyWidget("DamageLabel") );
		m_ItemQuantityLabel = TextWidget.Cast( layoutRoot.FindAnyWidget("QuantityLabel") );

		m_SpawnInInvButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("ButtonSpawnInInv") );
		m_SpawnGroundButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("ButtonSpawnInGround") );
		m_SpawnAsAttachment = ButtonWidget.Cast( layoutRoot.FindAnyWidget("SpawnAsAttachment") );
		m_SpawnSpecial = ButtonWidget.Cast( layoutRoot.FindAnyWidget("SpawnSpecial") );


		m_HelpTextListboxWidget = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("HelpTextListboxWidget") );

		PluginDeveloper module_dev = PluginDeveloper.Cast( GetPlugin(PluginDeveloper) );

		// config hierarchy
		m_ObjectConfigFilter = EditBoxWidget.Cast( layoutRoot.FindAnyWidget("ObjectConfigFilter") );
		m_ConfigHierarchyTextListbox = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("ConfigHierarchy") );
		m_ConfigVariablesTextListbox = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("ConfigVariables") );
		m_SelectedRowCopy = ButtonWidget.Cast( layoutRoot.FindAnyWidget("SelectedRowCopy") );

		m_CloseConsoleButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget("CloseConsoleButtonWidget") );

		m_MissionGameplay			= MissionGameplay.Cast( GetGame().GetMission() );
		m_Developer					= PluginDeveloper.Cast( GetPlugin(PluginDeveloper) );
		m_ConfigDebugProfile		= PluginConfigDebugProfile.Cast( GetPlugin(PluginConfigDebugProfile) );
		m_ConfigDebugProfileFixed	= PluginConfigDebugProfileFixed.Cast( GetPlugin(PluginConfigDebugProfileFixed) );
		m_ModuleConfigViewer		= PluginConfigViewer.Cast( GetPlugin( PluginConfigViewer ) );

		// Update checkbox Character Values
		m_HudDCharStats.SetChecked(m_ConfigDebugProfile.GetCharacterStatsVisible());
		m_HudDCharLevels.SetChecked(m_ConfigDebugProfile.GetCharacterLevelsVisible());
		m_HudDCharStomach.SetChecked(m_ConfigDebugProfile.GetCharacterStomachVisible());
		m_HudDCharModifiers.SetChecked(m_ConfigDebugProfile.GetCharacterModifiersVisible());
		m_HudDCharAgents.SetChecked(m_ConfigDebugProfile.GetCharacterAgentsVisible());
		m_HudDCharDebug.SetChecked(m_ConfigDebugProfile.GetCharacterDebugVisible());
		m_HudDFreeCamCross.SetChecked(m_ConfigDebugProfile.GetFreeCameraCrosshairVisible());
		m_HudDVersion.SetChecked(m_ConfigDebugProfile.GetVersionVisible());

		m_LogsEnabled.SetChecked(m_ConfigDebugProfile.GetLogsEnabled());

		ref TStringArray diag_names = new TStringArray;
		GetGame().GetDiagModeNames( diag_names );
		for ( i = 0; i < diag_names.Count(); i++)
		{
			m_DiagToggleTextListbox.AddItem(diag_names.Get(i), NULL, 0);
		}

		GetGame().GetDiagDrawModeNames( diag_names );
		for ( i = 0; i < diag_names.Count(); i++)
		{
			m_DiagDrawmodeTextListbox.AddItem(diag_names.Get(i), NULL, 0);
		}

		m_ObjectFilter.SetText( m_ConfigDebugProfile.GetItemSearch() );
		m_SpawnDistanceEditBox.SetText( m_ConfigDebugProfile.GetSpawnDistance().ToString() );
		ChangeFilter();

		m_Rows = 0;
		m_ServerRows = 0;
		m_EnscriptHistoryRow = 0;
		m_EnscriptHistoryRowServer = 0;

		m_EnscriptConsoleHistory = m_ModuleLocalEnscriptHistory.GetAllLines();
		m_EnscriptConsoleHistoryServer = m_ModuleLocalEnscriptHistoryServer.GetAllLines();

		// load data from profile
		m_selected_tab = m_ConfigDebugProfile.GetTabSelected();
		SelectTab(m_selected_tab);

		RenderPresets();
		HideItemButtons();

		m_PresetDeleteButton.Show( false );
		m_PresetRenameButton.Show( false );

		ShowItemTransferButtons();
		//EnscriptHistoryBack();

		
		TStringArray positions_array = new TStringArray;
		m_ConfigDebugProfileFixed.GetAllPositionsNames( positions_array );
		for ( i = 0; i < positions_array.Count(); i++ )
		{
			m_PositionsListbox.AddItem( positions_array.Get(i), new PresetParams ( positions_array.Get(i), true, false), 0);
		}
		
		ClearHierarchy();

		UpdateHudDebugSetting();

		m_DebugMapWidget.SetScale(DEBUG_MAP_ZOOM);
		m_DebugMapWidget.SetMapPos(GetGame().GetPlayer().GetWorldPosition());
		m_TeleportXYZ.SetText(DEFAULT_POS_XYZ);
		
		m_LateInit.Run(0.05, this, "LateInit", null, false);
		
		return layoutRoot;
	}

	void LateInit()
	{
		
		if(ITEMS_SELECTED_ROW <=  m_ObjectsTextListbox.GetNumItems() && m_ObjectsTextListbox.GetNumItems() >= 0 && ITEMS_SELECTED_ROW > -1 )
		{
			m_ObjectsTextListbox.SelectRow(ITEMS_SELECTED_ROW);
			m_ObjectsTextListbox.EnsureVisible(ITEMS_SELECTED_ROW);
			
			
			m_SelectedObjectText.SetText( GetCurrentObjectName() );

			m_SelectedObject = GetCurrentObjectName();
			m_LastSelectedObject = m_SelectedObject;
			m_PresetItemsTextListbox.SelectRow(-1);
			m_SelectedObjectIsPreset = false;
		}
		
		if (m_PresetsTextListbox.GetNumItems() >= 0 )
		{
			m_PresetsTextListbox.SelectRow(PRESETS_SELECTED_ROW);
			m_PresetsTextListbox.EnsureVisible(PRESETS_SELECTED_ROW);
		}
		
		if( m_ObjectsScope == 1 )
			m_ShowProtected.SetChecked(true);
		else
			m_ShowProtected.SetChecked(false);
		
		m_ShowOthers.SetChecked(SHOW_OTHERS);
		m_DrawDistanceWidget.SetText(DRAW_DISTANCE.ToString());
		
		RenderPresetItems();
		
		
		
		if(ITEMS_IN_PRESET_SELECTED_ROW <= m_PresetItemsTextListbox.GetNumItems() && m_PresetItemsTextListbox.GetNumItems() > 0 && ITEMS_IN_PRESET_SELECTED_ROW > -1 )
		{
			m_PresetItemsTextListbox.SelectRow(ITEMS_IN_PRESET_SELECTED_ROW);
			m_PresetItemsTextListbox.EnsureVisible(ITEMS_IN_PRESET_SELECTED_ROW);
			
			m_SelectedObjectText.SetText( "Object : " + GetCurrentItemName() );
			m_SelectedObject = GetCurrentItemName();
			m_SelectedObjectIsPreset = false;
		}
		
	}
	
	void Add(string message)
	{
		if (m_ClientLogListbox)
		{
			m_Rows = m_Rows + 1;

			if ( message != string.Empty )
			{
				m_ClientLogListbox.AddItem(String(message), NULL, 0);

				if ( m_ClientLogScrollCheckbox.IsChecked() )
				{
					m_ClientLogListbox.EnsureVisible( m_Rows );
				}
			}
		}
	}

	void AddServer(string message)
	{
		/*if (m_ServerLogListbox)
		{
			m_ServerRows = m_ServerRows + 1;
			m_ServerLogListbox.AddItem(String(message), NULL, 0);
			if ( m_ServerLogScrollCheckbox.IsChecked() )
			{
				m_ServerLogListbox.EnsureVisible( m_ServerRows );
			}
		}*/
	}
	
	void HistoryBack()
	{
		if ( m_ConfigDebugProfile.GetTabSelected() == ScriptConsole.TAB_ENSCRIPT || m_ConfigDebugProfile.GetTabSelected() == ScriptConsole.TAB_ENSCRIPT_SERVER )
		{
			EnscriptHistoryBack();
		}
	}

	void HistoryForward()
	{
		if ( m_ConfigDebugProfile.GetTabSelected() == ScriptConsole.TAB_ENSCRIPT || m_ConfigDebugProfile.GetTabSelected() == ScriptConsole.TAB_ENSCRIPT_SERVER)
		{
			EnscriptHistoryForward();
		}
	}
	
/*
	void EnscriptHistoryBack()
	{
		if ( m_EnfScriptEdit )
		{
			int history_index = m_EnscriptConsoleHistory.Count() - m_EnscriptHistoryRow - 1;
			if ( history_index > -1 )
			{
				m_EnfScriptEdit.SetText( m_EnscriptConsoleHistory.Get( history_index ) );
				m_EnscriptHistoryRow = m_EnscriptHistoryRow + 1;
			}
		}
	}	
	*/
	
	
	void EnscriptHistoryBack()
	{
		int history_index;
		
		if ( m_EnfScriptEdit )
		{
			if ( !m_ScriptServer )//client
			{
				m_EnscriptHistoryRow++;
				history_index = m_EnscriptConsoleHistory.Count() - m_EnscriptHistoryRow - 1;
				if ( history_index > -1 )
				{
					m_EnfScriptEdit.SetText( m_EnscriptConsoleHistory.Get( history_index ) );
				}
				else m_EnscriptHistoryRow--;
			}
			else//server
			{
				m_EnscriptHistoryRowServer++;
				history_index = m_EnscriptConsoleHistoryServer.Count() - m_EnscriptHistoryRowServer - 1;
				if ( history_index > -1 )
				{
					m_EnfScriptEdit.SetText( m_EnscriptConsoleHistoryServer.Get( history_index ) );
				}
				else m_EnscriptHistoryRowServer--;
			}
		}
	}
	
	
/*
	void EnscriptHistoryForward()
	{
		if ( m_EnfScriptEdit )
		{
			int history_index = m_EnscriptConsoleHistory.Count() - m_EnscriptHistoryRow + 1;
			if ( history_index < m_EnscriptConsoleHistory.Count() )
			{
				m_EnfScriptEdit.SetText( m_EnscriptConsoleHistory.Get( history_index ) );
				m_EnscriptHistoryRow = m_EnscriptHistoryRow - 1;
			}
		}
	}
	*/
	
	void EnscriptHistoryForward()
	{
		if ( m_EnfScriptEdit )
		{
			int history_index;
			if ( !m_ScriptServer )//client
			{
				m_EnscriptHistoryRow--;
				history_index = m_EnscriptConsoleHistory.Count() - m_EnscriptHistoryRow - 1;
				if ( history_index < m_EnscriptConsoleHistory.Count() )
				{
					m_EnfScriptEdit.SetText( m_EnscriptConsoleHistory.Get( history_index ) );
				}
				else m_EnscriptHistoryRow++;
				
			}
			else//server
			{
				m_EnscriptHistoryRowServer--;
				history_index = m_EnscriptConsoleHistoryServer.Count() - m_EnscriptHistoryRowServer - 1;
				if ( history_index < m_EnscriptConsoleHistoryServer.Count() )
				{
					m_EnfScriptEdit.SetText( m_EnscriptConsoleHistoryServer.Get( history_index ) );
				}
				else m_EnscriptHistoryRowServer++;
			}
			
		}
	}
	
	void Clear()
	{
		m_Rows = 0;
		m_ClientLogListbox.ClearItems();
	}
		
	void RunEnscript()
	{
		string code;
		m_EnfScriptEdit.GetText(code);
		GetGame().ExecuteEnforceScript("void scConsMain() \n{\n" + code + "\n}\n", "scConsMain");
		m_EnscriptConsoleHistory.Insert( code );
		m_ModuleLocalEnscriptHistory.AddNewLine( code );
	}	
	
	void RunEnscriptServer()
	{
		string code;
		m_EnfScriptEdit.GetText(code);
		m_EnscriptConsoleHistoryServer.Insert( code );
		m_ModuleLocalEnscriptHistoryServer.AddNewLine( code );
		CachedObjectsParams.PARAM1_STRING.param1 = code;
		GetGame().RPCSingleParam( null, ERPCs.DEV_RPC_SERVER_SCRIPT, CachedObjectsParams.PARAM1_STRING, true, GetGame().GetPlayer().GetIdentity() );
	}

	
	
	void ChangeFilter()
	{
		//! get text
		m_ConfigDebugProfile.SetItemSearch( m_ObjectFilter.GetText() );
		m_ObjectsTextListbox.ClearItems();

		string filter_lower = m_ObjectFilter.GetText().Trim();
		filter_lower.ToLower(  );

		
		ref TStringArray items = new TStringArray;

		ref TStringArray filters = new TStringArray;
		filter_lower.Split( " ", filters );

		ref TStringArray searching_in = new TStringArray;
		Debug.GetBaseConfigClasses( searching_in );

		for ( int i = 0; i < searching_in.Count(); i++ )
		{
			string config_path = searching_in.Get(i);

			int objects_count = GetGame().ConfigGetChildrenCount( config_path );
			for ( int j = 0; j < objects_count; j++ )
			{
				string child_name;
				GetGame().ConfigGetChildName( config_path, j, child_name );

				int scope = GetGame().ConfigGetInt( config_path + " " + child_name + " scope" );
				if ( scope >= m_ObjectsScope )
				{
					bool display = false;
					if ( filters.Count() > 0 )
					{
						string child_name_lower = child_name;
						child_name_lower.ToLower( );

						for ( int k = 0; k < filters.Count(); k++ )
						{
							if ( child_name_lower.Contains(filters.Get(k)))
							{
								display = true;
								break;
							}
						}
					}
					else
					{
						display = true;
					}

					if ( display )
					{
						items.Insert(child_name);
						
					}
				}
			}
		}
		items.Sort();
		foreach (string s: items)
		{
			m_ObjectsTextListbox.AddItem( s, NULL, 0 );
			
		}
			
	}

	void ChangeConfigFilter()
	{
		if ( m_ObjectConfigFilter.GetText() == "" )
		{
			ClearHierarchy();
		}
		else
		{
			FindInHierarchy( m_ObjectConfigFilter.GetText() );
		}
	}
	
	override bool OnKeyPress(Widget w, int x, int y, int key)
	{
		super.OnKeyPress(w, x, y, key);

		if ( w == m_ObjectFilter )
		{
			ChangeFilter();
		}
		else if ( w == m_ObjectConfigFilter )
		{
			ChangeConfigFilter();
		}

		return false;
	}
	
	override bool OnKeyDown(Widget w, int x, int y, int key)
	{
		
		if(key == KeyCode.KC_D && m_SelectedObject != "")
		{
			DrawItems(m_SelectedObject);
			return true;
		}
		else
			return super.OnKeyDown(w, x, y, key);
	}

	
	void DrawItems(string type)
	{	
		array<Object> objects = new array<Object>;
		array<CargoBase> proxies = new array<CargoBase>;
		
		GetGame().GetObjectsAtPosition(GetGame().GetPlayer().GetPosition(),DRAW_DISTANCE, objects, proxies);
		int i = 0;

		foreach (Object o: objects)
		{
			if (o.IsKindOf(type))
			{
				i++;
				//Shape shape = Shape.CreateSphere(COLOR_RED, ShapeFlags.TRANSP|ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER, o.GetPosition(), 1);
				vector lines[2];
				lines[0] = o.GetPosition();
				lines[1] = o.GetPosition() + "0 200 0";
				Shape line = Shape.CreateLines(COLOR_RED, ShapeFlags.TRANSP|ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER,lines,2);
				//m_DebugShapes.Insert(shape);
				m_DebugShapes.Insert(line);
			}
		}
		Print("found "+ i +" instances of type " + type);
	}
	
	
	void DrawItemsClear()
	{
		Print(m_DebugShapes.Count());
		foreach (Shape s: m_DebugShapes)
		{
			s.Destroy();
		}
		m_DebugShapes.Clear();
	}
	
	override void Update(float timeslice)
	{
		super.Update( timeslice );
		
		if( GetGame() && GetGame().GetInput() && GetGame().GetInput().LocalPress("UAUIBack", false) )
		{
			GetGame().GetUIManager().Back();
		}
		m_DebugMapWidget.ClearUserMarks();
		
		if(m_UpdateMap)
		{
			foreach(RemotePlayerStatDebug rpd: m_PlayerDebugStats)
			{
				if(rpd.m_Player != GetGame().GetPlayer())
				{
					vector dir = rpd.m_Pos - GetGame().GetPlayer().GetWorldPosition();
					dir[1] = 0;
					string dist = ((int)dir.Length()).ToString();
					string text = rpd.m_Name + " " +dist +"m.";
					m_DebugMapWidget.AddUserMark(rpd.m_Pos, text , COLOR_BLUE,"\\dz\\gear\\navigation\\data\\map_tree_ca.paa");
				}
			}
		}
		vector player_pos = GetGame().GetPlayer().GetWorldPosition();
		m_DebugMapWidget.AddUserMark(player_pos,"You", COLOR_RED,"\\dz\\gear\\navigation\\data\\map_tree_ca.paa");
		if(player_pos != GetMapPos())
			m_DebugMapWidget.AddUserMark(GetMapPos(),"Pos", COLOR_BLUE,"\\dz\\gear\\navigation\\data\\map_tree_ca.paa");
		UpdateMousePos();
		if(!m_PlayerPosRefreshBlocked)
			RefreshPlayerPosEditBoxes();
	}

	void UpdateMousePos()
	{
		int x,y;
		GetMousePos(x,y);
		vector mouse_pos, world_pos;
		mouse_pos[0] = x;
		mouse_pos[1] = y;
		world_pos = m_DebugMapWidget.ScreenToMap(mouse_pos);
		world_pos[1] = GetGame().SurfaceY(world_pos[0], world_pos[2]);
		vector player_pos = GetGame().GetPlayer().GetWorldPosition();
		//player_pos[1] = 0;
		float dst = (world_pos - player_pos).Length();
		if(m_MouseCurPos)
			m_MouseCurPos.SetText( "Mouse: "+ world_pos[0] +", "+ world_pos[1] +", "+ world_pos[2] + "|" + "(distance:"+dst.ToString()+")");
	}
		
	override bool OnController(Widget w, int control, int value)
	{
		super.OnController(w, control, value);
		
		if(control == ControlID.CID_TABLEFT && value != 0)
		{
			SelectPreviousTab();
		}
		
		if(control == ControlID.CID_TABRIGHT && value != 0)
		{
			SelectNextTab();
		}
	
		return true;
	}
	
	void SelectObject(bool hide_presets = true)
	{
		if(hide_presets)
			m_PresetItemsTextListbox.SelectRow(-1);
		HideItemButtons();
		ShowItemTransferButtons();
		m_SelectedObjectText.SetText( GetCurrentObjectName() );

		m_SelectedObject = GetCurrentObjectName();
		m_LastSelectedObject = m_SelectedObject;
		m_SelectedObjectIsPreset = false;
	}
		
	void SetMapPos(vector pos)
	{
		m_MapPos = pos;
		m_PlayerCurPos.SetText( "Pos: "+  pos[0].ToString() +", "+ pos[1].ToString() +", "+ pos[2].ToString() );
	}
	
	vector GetMapPos()
	{
		return m_MapPos;
	}
	
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		super.OnMouseButtonDown(w,x,y,button);
		
		if ( w ==  m_DebugMapWidget )
		{
			if(button == 0)
			{
				m_PlayerPosRefreshBlocked = true;
				int mouse_x,mouse_y;
				GetMousePos(mouse_x,mouse_y);
				vector mouse_pos, world_pos;
				mouse_pos[0] = mouse_x;
				mouse_pos[1] = mouse_y;
				world_pos = m_DebugMapWidget.ScreenToMap(mouse_pos);
				world_pos[1] = GetGame().SurfaceY(world_pos[0], world_pos[2]);
				
				SetMapPos(world_pos);
			}
			else if(button == 1)
			{
				SetMapPos(GetGame().GetPlayer().GetWorldPosition());
			}
		}
		return true;
	}
	
	
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

		int i;
		int objects_row_index;
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );

		PluginDeveloper module_dev = PluginDeveloper.Cast( GetPlugin(PluginDeveloper) );
		
		if ( w == m_ClientLogClearButton )
		{
			this.Clear();
			return true;
		}
		else if ( w == m_ConfigHierarchyTextListbox )
		{
			objects_row_index = m_ConfigHierarchyTextListbox.GetSelectedRow();
			//Print(objects_row_index);

			if ( objects_row_index > -1 && objects_row_index < m_ConfigHierarchyTextListbox.GetNumItems() )
			{
				ConfigParams config_params;
				m_ConfigHierarchyTextListbox.GetItemData( objects_row_index, 0, config_params );

				if ( config_params.param1 == false )
				{
					ExpandHierarchy( objects_row_index );
				}
				else
				{
					CollapseHierarchy( objects_row_index );
				}
				RenderVariables( objects_row_index );
			}
			return true;
		}
		else if ( w == m_PositionsListbox )
		{
			string position_name = GetCurrentPositionName();
			vector position = m_ConfigDebugProfileFixed.GetPositionByName( position_name );
			m_TeleportX.SetText( position[0].ToString() );
			m_TeleportY.SetText( position[2].ToString() );
			return true;
		}
		else if ( w == m_TeleportButton )
		{
			float pos_x = m_TeleportX.GetText().ToFloat();
			float pos_z = m_TeleportY.GetText().ToFloat();
			float pos_y = GetGame().SurfaceY(pos_x, pos_z);
			vector v = Vector(pos_x, pos_y, pos_z);
			string txt = m_TeleportXYZ.GetText();
			if(m_TeleportXYZ.GetText() != "" && m_TeleportXYZ.GetText() != DEFAULT_POS_XYZ)
			{
				string pos = m_TeleportXYZ.GetText();
				pos.Replace("<", "");
				pos.Replace(">", "");
				pos.Replace(",", "");
				v = pos.ToVector();
			}
			m_Developer.Teleport(player, v);
			return true;
		}
		else if ( w == m_ButtonCopyPos )
		{
			GetGame().CopyToClipboard( GetMapPos().ToString() );
		}
		else if ( w == m_LogsEnabled )
		{
			//Log("m_LogsEnabled: "+ToString(m_LogsEnabled.IsChecked()));

			if ( m_ConfigDebugProfile )
			{
				m_ConfigDebugProfile.SetLogsEnabled(m_LogsEnabled.IsChecked());
				LogManager.SetLogsEnabled(m_LogsEnabled.IsChecked());
			}

			return true;
		}
		else if ( w == m_HudDCharStats )
		{
			if ( m_ConfigDebugProfile )
			{
				m_ConfigDebugProfile.SetCharacterStatsVisible(m_HudDCharStats.IsChecked());
			}

			// Refresh UI by new settings
			m_MissionGameplay.GetHudDebug().RefreshByLocalProfile();

			return true;
		}
		else if ( w == m_HudDCharLevels )
		{
			if ( m_ConfigDebugProfile )
			{
				m_ConfigDebugProfile.SetCharacterLevelsVisible(m_HudDCharLevels.IsChecked());
			}

			// Refresh UI by new settings
			m_MissionGameplay.GetHudDebug().RefreshByLocalProfile();

			return true;
		}
		else if ( w == m_HudDCharStomach )
		{
			if ( m_ConfigDebugProfile )
			{
				m_ConfigDebugProfile.SetCharacterStomachVisible(m_HudDCharStomach.IsChecked());
			}

			// Refresh UI by new settings
			m_MissionGameplay.GetHudDebug().RefreshByLocalProfile();

			return true;
		}
		else if ( w == m_HudDVersion )
		{
			if ( m_ConfigDebugProfile )
			{
				m_ConfigDebugProfile.SetVersionVisible(m_HudDVersion.IsChecked());
			}

			// Refresh UI by new settings
			m_MissionGameplay.GetHudDebug().RefreshByLocalProfile();

			return true;
		}
		else if ( w == m_HudDCharModifiers )
		{
			if ( m_ConfigDebugProfile )
			{
				m_ConfigDebugProfile.SetCharacterModifiersVisible(m_HudDCharModifiers.IsChecked());
			}

			// Refresh UI by new settings
			m_MissionGameplay.GetHudDebug().RefreshByLocalProfile();

			return true;
		}
		else if ( w == m_HudDCharAgents )
		{
			if ( m_ConfigDebugProfile )
			{
				m_ConfigDebugProfile.SetCharacterAgentsVisible(m_HudDCharAgents.IsChecked());
			}

			// Refresh UI by new settings
			m_MissionGameplay.GetHudDebug().RefreshByLocalProfile();

			return true;
		}
		else if ( w == m_HudDCharDebug )
		{
			if ( m_ConfigDebugProfile )
			{
				m_ConfigDebugProfile.SetCharacterDebugVisible(m_HudDCharDebug.IsChecked());
			}

			// Refresh UI by new settings
			m_MissionGameplay.GetHudDebug().RefreshByLocalProfile();

			return true;
		}
		else if ( w == m_HudDFreeCamCross )
		{
			if ( m_ConfigDebugProfile )
			{
				m_ConfigDebugProfile.SetFreeCameraCrosshairVisible(m_HudDFreeCamCross.IsChecked());
			}

			// Refresh UI by new settings
			m_MissionGameplay.GetHudDebug().RefreshByLocalProfile();

			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(GetGame().GetMission().RefreshCrosshairVisibility);

			return true;
		}
		else if ( w == m_ObjectsTextListbox )
		{
			SelectObject();
			return true;
		}
		else if ( w == m_ObjectFilter )
		{
			ChangeFilter();
			return true;
		}
		else if ( w == m_SpawnInInvButton || w == m_SpawnGroundButton || w == m_SpawnAsAttachment || w == m_SpawnSpecial )
		{
			SaveProfileSpawnDistance();

			objects_row_index = m_ObjectsTextListbox.GetSelectedRow();
			
			vector rayStart = GetGame().GetCurrentCameraPosition();
			vector rayEnd = rayStart + GetGame().GetCurrentCameraDirection() * 1.5;		
			vector hitPos;
			vector hitNormal;
			int hitComponentIndex;		
			ref set<Object> hitObjects = new set<Object>;
			DayZPhysics.RaycastRV(rayStart, rayEnd, hitPos, hitNormal, hitComponentIndex, hitObjects, NULL, player);
					
			Object target = NULL;
			if( hitObjects.Count() )
				target = hitObjects.Get(0);
			
			if ( m_SelectedObject != "" )
			{
				// @NOTE: duplicate code in PluginDeveloper.c
				float distance = m_SpawnDistanceEditBox.GetText().ToFloat();
				if ( m_SelectedObjectIsPreset )
				{
					switch ( w )
					{
						case m_SpawnAsAttachment:
						{
							if ( target != NULL && target.IsInherited(EntityAI) )
							{
								EntityAI att_parent = EntityAI.Cast( target ) ;
								SpawnPreset(att_parent, CLEAR_IVN, m_SelectedObject);
							}
							else
							{
								SpawnPreset(player, false, m_SelectedObject);
							}
							break;
						}
						case m_SpawnGroundButton:
						{
							SpawnPreset(player, false, m_SelectedObject, InventoryLocationType.GROUND, distance);
							break;
						}
						
						default:
						{
							SpawnPreset(player, CLEAR_IVN, m_SelectedObject);
							break;
						}
					}
				}
				else
				{
					
					float health = m_DamageEditBox.GetText().ToFloat() * MiscGameplayFunctions.GetTypeMaxGlobalHealth( m_SelectedObject );
					float quantity = -1;
					int cfgQuantity = GetGame().ConfigGetInt("CfgVehicles " + m_SelectedObject + " varQuantityMax");

					if ( cfgQuantity > 0 )
					{
						quantity = m_QuantityEditBox.GetText().ToFloat() * GetGame().ConfigGetInt("CfgVehicles " + m_SelectedObject + " varQuantityMax");
						if ( GetGame().ConfigGetFloat("CfgVehicles " + m_SelectedObject + " canBeSplit") )
						{
							quantity = m_QuantityEditBox.GetText().ToFloat() * GetGame().ConfigGetInt("CfgVehicles " + m_SelectedObject + " varStackMax");
						}
					}

					switch ( w )
					{
						case m_SpawnGroundButton:
						{
							m_Developer.SpawnEntityOnCursorDir(player, m_SelectedObject, quantity, distance, health );
							break;
						}
						
						case m_SpawnSpecial:
						{
							m_Developer.SpawnEntityOnCursorDir(player, m_SelectedObject, quantity, distance, health, true);
							break;
						}

						case m_SpawnAsAttachment:
						{
							if ( target != NULL && target.IsInherited(EntityAI) )
							{
								EntityAI att_parent2 = EntityAI.Cast( target );
								m_Developer.SpawnEntityAsAttachment( player, att_parent2, m_SelectedObject, 100, -1);
							}
							else
							{
								m_Developer.SpawnEntityAsAttachment( player, player, m_SelectedObject, 100, -1);
							}
							break;
						}

						case m_SpawnInInvButton:
						{
							m_Developer.SpawnEntityInInventory(player, m_SelectedObject, -1, -1);
							break;
						}
					}
				}
			}
			return true;
		}
		else if ( w == m_DiagToggleButton )
		{
			int toggle_row_index = m_DiagToggleTextListbox.GetSelectedRow();
			bool toggle_state = GetGame().GetDiagModeEnable( toggle_row_index );
			GetGame().SetDiagModeEnable( toggle_row_index, !toggle_state );
			return true;
		}
		else if ( w == m_DiagDrawButton )
		{
			int draw_row_index = m_DiagDrawmodeTextListbox.GetSelectedRow();
			GetGame().SetDiagDrawMode( draw_row_index );
			return true;
		}
		else if ( w == m_CloseConsoleButton )
		{		
			Close();
			GetGame().GetMission().PlayerControlEnable(true);
			return true;
		}
		else if ( w == m_PresetsTextListbox )
		{
			SelectPreset();
			return true;
		}
		else if ( w == m_PresetItemsTextListbox )
		{
			m_ObjectsTextListbox.SelectRow(-1);
			ShowItemButtons();
			ShowItemTransferButtons();
			m_SelectedObjectText.SetText( "Object : " + GetCurrentItemName() );
			m_SelectedObject = GetCurrentItemName();
			m_SelectedObjectIsPreset = false;

			return true;
		}
		else if ( w == m_PresetSetDefaultButton )
		{
			if ( GetCurrentPresetName()!= "" )
			{
				SetDefaultPreset( GetCurrentPresetIndex() );
			}
			return true;
		}
		else if ( w == m_PresetNewButton )
		{
			g_Game.GetUIManager().EnterScriptedMenu(MENU_SCRIPTCONSOLE_DIALOG_PRESET_NAME, this);
			return true;
		}
		else if ( w == m_PresetDeleteButton )
		{
			if ( GetCurrentPresetName()!= "" )
			{
				DeletePreset();
			}
			return true;
		}
		else if ( w == m_PresetRenameButton )
		{
			if ( GetCurrentPresetName()!= "" )
			{
				g_Game.GetUIManager().EnterScriptedMenu(MENU_SCRIPTCONSOLE_DIALOG_PRESET_RENAME, this);
			}
			return true;
		}
		else if ( w == m_PresetAddItemtButton )
		{
			AddItemToPreset();
			return true;
		}
		else if ( w == m_ItemNameCopy )
		{
			AddItemToClipboard(m_ObjectsTextListbox);
			return true;
		}
		else if ( w == m_SelectedRowCopy )
		{
			AddItemToClipboard(m_ConfigVariablesTextListbox);
			return true;
		}
		else if ( w == m_PresetRemoveItemButton )
		{
			RemoveItemFromPreset();
			return true;
		}
		else if ( w == m_ItemMoveUpButton )
		{
			ItemMoveUp();
			return true;
		}
		else if ( w == m_ItemMoveDownButton )
		{
			ItemMoveDown();
			return true;
		}
		else if (w == m_EnfScriptRun)
		{
			if ( m_ScriptServer)
				RunEnscriptServer();
			else
				RunEnscript();
			return true;
		}
		else if (w == m_DrawInWorld)
		{
			DrawItems(m_SelectedObject);
			return true;
			
		}
		else if (w == m_DrawInWorldClear)
		{
			DrawItemsClear();
			return true;
		}
		
		// tabs
		for (i = 0; i < TABS_COUNT; i++)
		{
			if (w == m_Tab_buttons[i])
			{
				SelectTab(i);
				return true;
			}
		}

		return false;
	}
	
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		super.OnDoubleClick(w, x, y, button);

		int i;
		int objects_row_index;
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );
		
		if ( w == m_ObjectsTextListbox )
		{
			//select item
			m_PresetItemsTextListbox.SelectRow(-1);
			HideItemButtons();
			ShowItemTransferButtons();
			m_SelectedObjectText.SetText( "Object : " + GetCurrentObjectName() );

			m_SelectedObject = GetCurrentObjectName();
			m_LastSelectedObject = m_SelectedObject;
			m_SelectedObjectIsPreset = false;
			
			//spawn item
			SaveProfileSpawnDistance();

			objects_row_index = m_ObjectsTextListbox.GetSelectedRow();
			
			vector rayStart = GetGame().GetCurrentCameraPosition();
			vector rayEnd = rayStart + GetGame().GetCurrentCameraDirection() * 1.5;		
			vector hitPos;
			vector hitNormal;
			int hitComponentIndex;		
			ref set<Object> hitObjects = new set<Object>;
			DayZPhysics.RaycastRV(rayStart, rayEnd, hitPos, hitNormal, hitComponentIndex, hitObjects, NULL, player);
					
			Object target = NULL;
			if( hitObjects.Count() )
				target = hitObjects.Get(0);
			
			if ( m_SelectedObject != "" )
			{
				float distance = m_SpawnDistanceEditBox.GetText().ToFloat();
				
				if ( button == 0 )		//LMB
				{
					m_Developer.SpawnEntityOnCursorDir(player, m_SelectedObject, 100, distance);
				}
				else if ( button == 1 )		//RMB
				{
					if ( GetGame().IsMultiplayer() )
					{
						m_Developer.SpawnEntityInInventory(player, m_SelectedObject, -1, -1);
					}
					else
					{
						EntityAI spawned_entity = m_Developer.SpawnEntityOnCursorDir(player, m_SelectedObject, -1, distance );
						player.LocalTakeEntityToHands(spawned_entity);
					}
				}
			}
		}
		
		if ( w ==  m_DebugMapWidget )
		{
			vector screen_to_map = m_DebugMapWidget.ScreenToMap(Vector(x,y, 0));
			float pos_y = GetGame().SurfaceY(screen_to_map[0], screen_to_map[2]);
			screen_to_map[1] = pos_y;
			m_Developer.Teleport(player, screen_to_map);
			return true;
		}
		

		return false;
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		
		if(w == m_TeleportXYZ/* || w == m_TeleportX ||w == m_TeleportY*/)
		{
			if(m_TeleportXYZ.GetText() == "")
				EditBoxWidget.Cast(w).SetText(DEFAULT_POS_XYZ);
			return false;
		}
		return false;
	}
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);
		
		if(w == m_TeleportXYZ/* || w == m_TeleportX ||w == m_TeleportY*/)
		{
			EditBoxWidget.Cast(w).SetText("");
			return false;
		}
		
		return false;
	}
	


	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		super.OnChange(w, x, y, finished);

		if (w == m_ObjectFilter)
		{
			ChangeFilter();
			return true;
		}
		else if (w == m_QuantityEditBox && ( GetCurrentItemIndex() >= 0 || GetCurrentPresetName() != "") )
		{
			m_ConfigDebugProfile.SetItemQuantity( GetCurrentPresetName(), GetCurrentItemIndex(), m_QuantityEditBox.GetText().ToFloat() );
			return true;
		}
		else if (w == m_DrawDistanceWidget)
		{
			DRAW_DISTANCE = m_DrawDistanceWidget.GetText().ToFloat();
			return true;
		}
		else if (w == m_DamageEditBox && ( GetCurrentItemIndex() >= 0 || GetCurrentPresetName() != "") )
		{
			m_ConfigDebugProfile.SetItemHealth( GetCurrentPresetName(), GetCurrentItemIndex(), m_DamageEditBox.GetText().ToFloat() );
			return true;
		}
		else if ( w == m_ObjectConfigFilter )
		{
			ChangeConfigFilter();
			return true;
		}
		else if ( w == m_ShowProtected )
		{
			if( m_ShowProtected.IsChecked() )
			{
				m_ObjectsScope = 1;	
			}
			else
			{
				m_ObjectsScope = 2;
			}
			ChangeFilter();
			return true;
		}
		else if ( w == m_ShowOthers )
		{
			PluginRemotePlayerDebugClient plugin_remote_client = PluginRemotePlayerDebugClient.Cast( GetPlugin(PluginRemotePlayerDebugClient) );
			if( m_ShowOthers.IsChecked() )
			{
				plugin_remote_client.RequestPlayerInfo( PlayerBase.Cast(GetGame().GetPlayer()), 1 );
				m_UpdateMap = 1;
				SHOW_OTHERS = true;
			}
			else
			{
				plugin_remote_client.RequestPlayerInfo( PlayerBase.Cast(GetGame().GetPlayer()), 0 );
				m_UpdateMap = 0;
				SHOW_OTHERS = false;
			}
			return true;
		}
		else if ( w == m_ClearInventory )
		{
			CLEAR_IVN = m_ClearInventory.IsChecked();
			return true;
		}
		
		

		return false;
	}
	
	override bool OnItemSelected(Widget w, int x, int y, int row, int  column,	int  oldRow, int  oldColumn)
	{
		super.OnItemSelected(w, x, y, row, column, oldRow, oldColumn);
		
		if ( w == m_PositionsListbox )
		{
			vector position = m_ConfigDebugProfileFixed.GetPositionByName( GetCurrentPositionName() );
			m_TeleportX.SetText( position[0].ToString() );
			m_TeleportY.SetText( position[2].ToString() );
			return true;
		}
		else if( w == m_ObjectsTextListbox )
		{
			SelectObject(false);
			return true;
		}
		else if ( w == m_PresetItemsTextListbox )
		{
			if(m_PresetItemsTextListbox.GetSelectedRow() != -1)
			{
				ShowItemButtons();
				ShowItemTransferButtons();
				m_SelectedObjectText.SetText( "Object : " + GetCurrentItemName() );
				m_SelectedObject = GetCurrentItemName();
				m_SelectedObjectIsPreset = false;
			}
			return true;
		}
		else if ( w == m_PresetsTextListbox )
		{
			if(m_PresetsTextListbox.GetSelectedRow() != -1)
			{
				SelectPreset();
				return true;
			}
		}
		return false;
	}

	void SelectPreset()
	{
		RenderPresetItems();
		ShowPresetButtons();
		ShowItemTransferButtons();
		m_SelectedObject = GetCurrentPresetName();
		m_SelectedObjectText.SetText( "Preset : " + m_SelectedObject );
		m_SelectedObjectIsPreset = true;
		
		TStringArray command_array = new TStringArray;
		
		m_ConfigDebugProfileFixed.GetPresetItems( m_SelectedObject, command_array, "cmd" );
				
		bool clear = false;
		if(command_array.IsValidIndex(0))
		{
			string first_line = command_array.Get(0);
			first_line.ToLower();
			clear = (first_line == "clear_inv");
		}
		m_ClearInventory.SetChecked(clear);
	}
	
	void RefreshLists()
	{
		RenderPresets();
		RenderPresetItems();
	}
	
	// Render specific Preset Items
	void RenderPresets()
	{
		m_PresetsTextListbox.ClearItems();

		int i;
		TBoolArray preset_params;

		// load fixed presets list
		TStringArray presets_array = m_ConfigDebugProfileFixed.GetPresets();
		for ( i = 0; i < presets_array.Count(); i++ )
		{
			m_PresetsTextListbox.AddItem( "["+presets_array.Get(i)+"]", new PresetParams ( presets_array.Get(i), true, false), 0);
		}

		// load custom presets list
		TStringArray custom_presets_array = m_ConfigDebugProfile.GetPresets();
		for ( i = 0; i < custom_presets_array.Count(); i++ )
		{
			m_PresetsTextListbox.AddItem( custom_presets_array.Get(i), new PresetParams ( custom_presets_array.Get(i),false, false), 0);
		}

		string default_preset = m_ConfigDebugProfile.GetDefaultPreset();
		if ( default_preset != "" )
		{
			// if is fixed
			int index = GetPresetIndexByName( default_preset );
			if ( IsPresetFixed( default_preset) )
			{
				default_preset = "[" + default_preset + "]";
			}
			PresetParams preset_params_array;
			if( index > -1 && index < m_PresetsTextListbox.GetNumItems() )
			{
				m_PresetsTextListbox.GetItemData( index, 0, preset_params_array );
				m_PresetsTextListbox.SetItem( index, default_preset + CONST_DEFAULT_PRESET_PREFIX, preset_params_array, 0 );
			}
		}
	}
	
	// Render specific Preset Items
	void RenderPresetItems()
	{
		// load preset items list
		int i;
		m_PresetItemsTextListbox.ClearItems();
		if ( GetCurrentPresetIndex() != -1 )
		{
			bool isFixed = IsCurrentPresetFixed();
			TStringArray preset_array = new TStringArray;

			if ( isFixed )
			{
				 m_ConfigDebugProfileFixed.GetPresetItems( GetCurrentPresetName(), preset_array );
			}
			else
			{
				m_ConfigDebugProfile.GetPresetItems( GetCurrentPresetName(), preset_array );
			}

			if ( preset_array )
			{
				for ( i = 0; i < preset_array.Count(); i++)
				{
					m_PresetItemsTextListbox.AddItem( preset_array.Get(i), NULL, 0);
				}
			}
		}
	}
	
	void NewPreset( string preset_name )
	{
		m_ConfigDebugProfile.PresetAdd( preset_name );
		RefreshLists();
	}

	void DeletePreset()
	{
		if ( GetCurrentPresetIndex() != -1 )
		{
			bool result = m_ConfigDebugProfile.PresetRemove( GetCurrentPresetName() );
			RefreshLists();
		}
	}
	
	void SaveProfileSpawnDistance()
	{
		if ( m_ConfigDebugProfile && m_SpawnDistanceEditBox )
		{
			m_ConfigDebugProfile.SetSpawnDistance( m_SpawnDistanceEditBox.GetText().ToFloat() );
		}
	}
	
	void RenamePreset( string new_preset_name )
	{
		if ( GetCurrentPresetIndex() != -1 )
		{
			bool result = m_ConfigDebugProfile.PresetRename( GetCurrentPresetName(), new_preset_name );
			RefreshLists();
		}
	}
	
	
	void SpawnPreset(EntityAI target, bool clear_inventory, string preset_name, InventoryLocationType location = InventoryLocationType.ATTACHMENT, float distance = 0)
	{
		// spawn preset items into inventory
		int i;
		if ( GetCurrentPresetIndex() != -1 )
		{
			bool is_preset_fixed = IsCurrentPresetFixed();
			TStringArray preset_array = new TStringArray;
			
			if ( is_preset_fixed )
			{
				m_ConfigDebugProfileFixed.GetPresetItems( preset_name, preset_array );
				
			}
			else
			{
				m_ConfigDebugProfile.GetPresetItems( preset_name, preset_array );
			}
			
			if ( clear_inventory )
			{
				PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );
				m_Developer.ClearInventory(player);
			}

			for ( i = 0; i < preset_array.Count(); i++)
			{
				float health = -1;
				int quantity = -1;
				if ( is_preset_fixed )
				{
					health = m_ConfigDebugProfileFixed.GetItemHealth( preset_name, i );
					quantity = m_ConfigDebugProfileFixed.GetItemQuantity( preset_name, i );
				}
				else
				{
					health = m_ConfigDebugProfile.GetItemHealth( preset_name, i );
					quantity = m_ConfigDebugProfile.GetItemQuantity( preset_name, i );
				}
				if(location == InventoryLocationType.ATTACHMENT)
					EntityAI ent = m_Developer.SpawnEntityInInventory(PlayerBase.Cast( target ), preset_array.Get(i), -1, quantity, true);
				else if(location == InventoryLocationType.GROUND)
					m_Developer.SpawnEntityOnCursorDir(PlayerBase.Cast( target ), preset_array.Get(i), quantity, distance, health );
			}
		}
	}
	
	void SetDefaultPreset( int preset_index )
	{
		// remove previous default parameter
		string default_preset = m_ConfigDebugProfile.GetDefaultPreset();
		if ( default_preset != "" )
		{
			int index = GetPresetIndexByName( default_preset );
			// if is fixed
			if ( IsPresetFixed( default_preset) )
			{
				default_preset = "[" + default_preset + "]";
			}
			PresetParams prev_preset_params_array;
			if( index > -1 && index < m_PresetsTextListbox.GetNumItems() )
			{
				m_PresetsTextListbox.GetItemData( index, 0, prev_preset_params_array );
				prev_preset_params_array.param3 = false; // remove DEFAULT
				m_PresetsTextListbox.SetItem( index, default_preset, prev_preset_params_array, 0 );
			}
		}

		// set preset on preset_index to default
		// if is fixed
		string preset_name = GetCurrentPresetName();
		if ( IsPresetFixed( preset_name) )
		{
			preset_name = "[" + preset_name + "]";
		}
		// set new default preset
		PresetParams preset_params_array;
		index = GetCurrentPresetIndex();
		if ( index > -1 && index < m_PresetsTextListbox.GetNumItems() )
		{
			m_PresetsTextListbox.GetItemData( index, 0, preset_params_array );
			preset_params_array.param3 = true; // DEFAULT
			m_PresetsTextListbox.SetItem( index, preset_name + CONST_DEFAULT_PRESET_PREFIX, preset_params_array, 0 );
		}
		// store preset
		m_ConfigDebugProfile.SetDefaultPreset( GetCurrentPresetName() );
	}
	
	void AddItemToPreset()
	{
		int selected_row_index = m_ObjectsTextListbox.GetSelectedRow();
		if ( selected_row_index != -1 && GetCurrentPresetIndex() != -1 )
		{
			string item_name;
			m_ObjectsTextListbox.GetItemText( selected_row_index, 0, item_name );
			m_ConfigDebugProfile.ItemAddToPreset( GetCurrentPresetName(), item_name);
			RenderPresetItems();
		}
	}
	
	void ShowPresetButtons()
	{
		bool show = true;
		if ( IsCurrentPresetFixed() )
		{
			show = false;
		}
		m_PresetDeleteButton.Show( show );
		m_PresetRenameButton.Show( show );

		HideItemButtons();
		
	}

	void AddItemToClipboard( TextListboxWidget text_listbox_widget )
	{
		int selected_row_index = text_listbox_widget.GetSelectedRow();
		if ( selected_row_index != -1 )
		{
			string item_name;
			text_listbox_widget.GetItemText( selected_row_index, 0, item_name );
			GetGame().CopyToClipboard( item_name );
		}
	}

	void RemoveItemFromPreset()
	{
		if ( GetCurrentItemIndex() != -1 && GetCurrentPresetIndex() != -1 )
		{
			m_ConfigDebugProfile.ItemRemoveFromPreset( GetCurrentPresetName(), GetCurrentItemIndex() );
			RenderPresetItems();
		}
	}
	
	string GetCurrentPresetName()
	{
		int index = GetCurrentPresetIndex();
		// load preset items list
		if ( index > -1 && index < m_PresetsTextListbox.GetNumItems() )
		{
			PresetParams item_params_array;
			m_PresetsTextListbox.GetItemData( index, 0, item_params_array );
			return item_params_array.param1;
		}
		return "";
	}
	
	string GetCurrentItemName()
	{
		if ( GetCurrentItemIndex() != -1 )
		{
			string item_name;
			m_PresetItemsTextListbox.GetItemText( GetCurrentItemIndex(), 0, item_name );
			return item_name;
		}
		return "";
	}
	
	string GetCurrentObjectName()
	{
		int selected_row_index = m_ObjectsTextListbox.GetSelectedRow();
		if ( selected_row_index != -1 )
		{
			string item_name;
			m_ObjectsTextListbox.GetItemText( selected_row_index, 0, item_name );
			return item_name;
		}
		return "";
	}
	
	string GetCurrentPositionName()
	{
		if ( m_PositionsListbox.GetSelectedRow() != -1 )
		{
			string position_name;
			m_PositionsListbox.GetItemText( m_PositionsListbox.GetSelectedRow(), 0, position_name );
			return position_name;
		}
		return "";
	}
	
	int GetCurrentPresetIndex()
	{
		return m_PresetsTextListbox.GetSelectedRow();
	}

	int GetCurrentItemIndex()
	{
		return m_PresetItemsTextListbox.GetSelectedRow();
	}
	
	bool IsCurrentPresetFixed()
	{
		int index = GetCurrentPresetIndex(); 
		if ( index > -1 && index < m_PresetsTextListbox.GetNumItems() )
		{
			PresetParams item_params_array;
			m_PresetsTextListbox.GetItemData( index, 0, item_params_array );
			return item_params_array.param2;
		}
		return false;
	}

	bool IsPresetFixed( string preset_name )
	{
		int preset_index = GetPresetIndexByName( preset_name);
		PresetParams item_params_array;
		if ( preset_index > -1 && preset_index < m_PresetsTextListbox.GetNumItems() )
		{
			m_PresetsTextListbox.GetItemData( preset_index, 0, item_params_array );
			return item_params_array.param2;
		}
		return false;
	}
	
	int GetPresetIndexByName( string preset_name )
	{
		int i;
		for ( i = 0; i < m_PresetsTextListbox.GetNumItems(); i++ )
		{
			PresetParams item_params_array;
			m_PresetsTextListbox.GetItemData( i, 0, item_params_array );

			if ( item_params_array.param1 == preset_name )
			{
				return i;
			}
		}
		return -1;
	}
	
	void ShowItemButtons()
	{
		if ( !IsCurrentPresetFixed() )
		{
			ShowItemTransferButtons();

			string preset_name = GetCurrentPresetName();
			int item_index = GetCurrentItemIndex();

			m_ItemMoveUpButton.Show(true);
			m_ItemMoveDownButton.Show(true);

			m_ItemDamageLabel.Show( true );
			m_ItemQuantityLabel.Show( true );

			if ( preset_name != "" && item_index > -1 )
			{
				m_DamageEditBox.Show( true );
				m_QuantityEditBox.Show( true );
				float item_health= m_ConfigDebugProfile.GetItemHealth( GetCurrentPresetName(), GetCurrentItemIndex() );
				int item_quantity = m_ConfigDebugProfile.GetItemQuantity( GetCurrentPresetName(), GetCurrentItemIndex() );

				// damage
				m_DamageEditBox.SetText( item_health.ToString() );

				// quantity
				m_QuantityEditBox.SetText( item_quantity.ToString() );
				// int stacked_max = GetGame().ConfigGetInt( CFG_VEHICLESPATH + " " + GetCurrentItemName() + " stackedMax");
				// m_QuantityEditBox.SetText( itoa( stacked_max ) );
			}
			else
			{
				m_DamageEditBox.Show( false );
				m_QuantityEditBox.Show( false );
			}
		}
	}

	void HideItemButtons()
	{
		m_ItemMoveUpButton.Show( false );
		m_ItemMoveDownButton.Show( false );

//		m_DamageEditBox.Show( false );
//		m_QuantityEditBox.Show( false );
//		m_ItemDamageLabel.Show( false );
//		m_ItemQuantityLabel.Show( false );
	}
		
	void ShowItemTransferButtons()
	{
		int object_row = m_ObjectsTextListbox.GetSelectedRow();
		if ( object_row >-1 && GetCurrentPresetIndex() >-1 && !IsCurrentPresetFixed() )
		{
			m_PresetAddItemtButton.Show( true );
		}
		else
		{
			m_PresetAddItemtButton.Show( false );
		}

		if ( GetCurrentItemIndex() >-1 && GetCurrentPresetIndex() >-1 && !IsCurrentPresetFixed() )
		{
			m_PresetRemoveItemButton.Show( true );
		}
		else
		{
			m_PresetRemoveItemButton.Show( false );
		}

		if ( object_row > -1 )
		{
			m_ItemNameCopy.Show( true );
		}
		else
		{
			m_ItemNameCopy.Show( false );
		}

	}
	
	void ItemMoveUp()
	{
		int new_index = GetCurrentItemIndex() - 1;
		if ( GetCurrentItemIndex() != -1 && GetCurrentPresetIndex() != -1 && new_index > -1)
		{
			m_ConfigDebugProfile.SwapItem( GetCurrentPresetName(), GetCurrentItemIndex(), new_index );
			RenderPresetItems();
			m_PresetItemsTextListbox.SelectRow (new_index);
		}
	}

	void ItemMoveDown()
	{
		int new_index = GetCurrentItemIndex() + 1;
		if ( GetCurrentItemIndex() != -1 && GetCurrentPresetIndex() != -1 && new_index < m_PresetItemsTextListbox.GetNumItems() )
		{
			m_ConfigDebugProfile.SwapItem( GetCurrentPresetName(), GetCurrentItemIndex(), new_index );
			RenderPresetItems();
			m_PresetItemsTextListbox.SelectRow (new_index);
		}
	}

	void ClearHierarchy()
	{
		// config hierarchy
		// string config_path = "configfile CfgVehicles APC";
		// string config_path = "";

		m_ConfigHierarchyTextListbox.ClearItems();
		m_ConfigVariablesTextListbox.ClearItems();

		ref TStringArray base_classes = new TStringArray;
		Debug.GetBaseConfigClasses( base_classes );

		string config_path = "configfile";
		ref TStringArray variables = m_ModuleConfigViewer.GetConfigHierarchy( config_path );
		for ( int i = 0; i < variables.Count(); i++ )
		{
			string variable = variables.Get(i);

			for ( int j = 0; j < base_classes.Count(); j++ )
			{
				if ( variable == base_classes.Get(j) )
				{
					string new_config_path = ( config_path + " " + variable ).Trim();
					m_ConfigHierarchyTextListbox.AddItem( "+ " + variable, new ConfigParams( false, variable, 0, new_config_path, 0 ), 0);
				}
			}
		}
	}

	void FindInHierarchy( string class_name )
	{
		m_ConfigHierarchyTextListbox.ClearItems();
		m_ConfigVariablesTextListbox.ClearItems();

		class_name.ToLower(  );
		string config_base_path = "configfile";

		ref TStringArray base_classes = new TStringArray;
		Debug.GetBaseConfigClasses( base_classes );

		string filter_lower = class_name;
		filter_lower.ToLower(  );

		ref TStringArray filters = new TStringArray;
		filter_lower.Split( " ", filters );

		for ( int i = 0; i < base_classes.Count(); i++ )
		{
			string config_path = config_base_path + " " + base_classes.Get(i);
			ref TStringArray variables = m_ModuleConfigViewer.GetConfigHierarchy( config_path );

			for ( int j = 0; j < variables.Count(); j++ )
			{
				string variable = variables.Get(j);
				string variable_lower = variable;
				variable_lower.ToLower();

				for ( int k = 0; k < filters.Count(); k++ )
				{
					if ( variable_lower.Contains(filters.Get(k)))
					{
						string new_config_path = ( config_path + " " + variable ).Trim();
						m_ConfigHierarchyTextListbox.AddItem( "+ " + variable, new ConfigParams( false, variable, 0, new_config_path, 0 ), 0);
						break;
					}
				}
			}
		}
	}
	
	void ExpandHierarchy( int row )
	{
		if( row <= -1 && row >= m_ConfigHierarchyTextListbox.GetNumItems() )
		{
			return;
		}
		
		ref TStringArray variables;

		// get current row data
		ConfigParams config_params;
		m_ConfigHierarchyTextListbox.GetItemData( row, 0, config_params );

		string config_path = config_params.param4;
		int deep = config_params.param5;

		string offset = "";
		for ( int i = 0; i < deep; i++)
		{
			offset = offset + "  ";
		}

		// change selected node
		variables = m_ModuleConfigViewer.GetConfigHierarchy( config_path );
		int childrens_count = variables.Count();
		m_ConfigHierarchyTextListbox.SetItem( row, offset + "- " + config_params.param2, new ConfigParams( true, config_params.param2, childrens_count, config_path, deep ), 0 );

		offset = offset + "  ";

		// render childrens
		deep = deep + 1;
		childrens_count = 0;
		for ( i = variables.Count() - 1; i >= 0; i-- )
		{
			string new_config_path = ( config_path + " " + variables.Get(i) ).Trim();
			m_ConfigHierarchyTextListbox.AddItem( offset + "+ " + variables.Get(i), new ConfigParams( false, variables.Get(i), childrens_count, new_config_path, deep  ), 0, (row + 1) );
		}
	}

	// niekde je bug, ked su len 2 polozky a expand/collapse na prvu tak zmaze aj druhu. ak collapse a expand na druhu tak crash lebo sa snazi zmazat riadok co neexistuje
	void CollapseHierarchy( int row )
	{
		if( row <= -1 && row >= m_ConfigHierarchyTextListbox.GetNumItems() )
		{
			return;
		}
		
		ConfigParams config_params;
		ConfigParams config_params_next;
		m_ConfigHierarchyTextListbox.GetItemData( row, 0, config_params );
		m_ConfigHierarchyTextListbox.GetItemData( row + 1, 0, config_params_next );

		int deep = config_params.param5;
		int deep_next = config_params_next.param5;
		int max_count = m_ConfigHierarchyTextListbox.GetNumItems();
		int remove_lines_count = 0;
		// Print(max_count);
		for ( int i = row + 1; i < max_count; i++)
		{
			if ( deep < deep_next && i <= max_count )
			{
				remove_lines_count = remove_lines_count + 1;
				m_ConfigHierarchyTextListbox.GetItemData( i, 0, config_params_next );
				deep_next = config_params_next.param5;
			}
		}

		// remove remove_lines_count lines from row
		// remove_lines_count = remove_lines_count - 1;
		for ( i = 1; i < remove_lines_count; i++ )
		{
			int x = row + 1;
			if ( x < max_count )
			{
				m_ConfigHierarchyTextListbox.RemoveRow( x );
			}
		}

		string offset = "";
		for ( i = 0; i < deep; i++)
		{
			offset = offset + "  ";
		}
		m_ConfigHierarchyTextListbox.SetItem( row, offset + "+ " + config_params.param2, new ConfigParams( false, config_params.param2, 0, config_params.param4, deep ), 0 );
		
		if( deep == 0 )
		{
			ClearHierarchy();
		}
	}
	
	void RenderVariables( int row )
	{
		ConfigParams config_params;
		if( row > -1 && row < m_ConfigHierarchyTextListbox.GetNumItems() )
		{
			m_ConfigHierarchyTextListbox.GetItemData( row, 0, config_params );
			m_ConfigVariablesTextListbox.ClearItems();
			ref TStringArray variables;
			string path = config_params.param4;
			variables = m_ModuleConfigViewer.GetConfigVariables( path );
			for ( int i = 0; i < variables.Count(); i++ )
			{
				m_ConfigVariablesTextListbox.AddItem( variables.Get(i), NULL, 0);
			}
		}
	}
	
	void UpdateHudDebugSetting();
	
	void RefreshPlayerPosEditBoxes()
	{
		//if ( GetGame() != NULL && GetGame().GetPlayer() != NULL )
		{
			vector player_pos = GetGame().GetPlayer().GetPosition();
			SetMapPos(player_pos);
			//m_PlayerCurPos.SetText( "Pos: "+ player_pos[0] +", "+ player_pos[1] +", "+ player_pos[2] );
		}
	}

	//!
	static string	GetLastSelectedObject()
	{
		return m_LastSelectedObject;
	}

	//private ref Timer m_TimerRefreshPlayerPosEditBoxes = NULL;

	int m_selected_tab;
	int m_Rows;
	int m_ServerRows;
	int m_EnscriptHistoryRow;
	int m_EnscriptHistoryRowServer;
	ref TStringArray		m_EnscriptConsoleHistory;
	ref TStringArray		m_EnscriptConsoleHistoryServer;
	PluginLocalEnscriptHistory	m_ModuleLocalEnscriptHistory;
	PluginLocalEnscriptHistoryServer	m_ModuleLocalEnscriptHistoryServer;
	MissionGameplay				m_MissionGameplay;

	TextListboxWidget m_ClientLogListbox;
	ButtonWidget m_ClientLogClearButton;
	CheckBoxWidget m_ClientLogScrollCheckbox;

	TextListboxWidget m_DiagToggleTextListbox;
	ButtonWidget m_DiagToggleButton;

	TextListboxWidget m_DiagDrawmodeTextListbox;
	ButtonWidget m_DiagDrawButton;
	ButtonWidget m_DrawInWorld;
	ButtonWidget m_DrawInWorldClear;
	// Page EnfScript
	MultilineEditBoxWidget m_EnfScriptEdit;
	ButtonWidget m_EnfScriptRun;
	ButtonWidget m_EnfScriptRunServer;

	// Page General
	ButtonWidget 		m_TeleportButton;
	ButtonWidget 		m_ButtonCopyPos;
	EditBoxWidget 		m_TeleportX;
	EditBoxWidget 		m_TeleportY;
	EditBoxWidget 		m_TeleportXYZ;
	TextWidget	 		m_PlayerCurPos;
	TextWidget	 		m_MouseCurPos;
	TextListboxWidget	m_PositionsListbox;
	CheckBoxWidget		m_LogsEnabled;
	CheckBoxWidget		m_HudDCharStats;
	CheckBoxWidget		m_HudDCharLevels;
	CheckBoxWidget		m_HudDCharStomach;
	CheckBoxWidget		m_HudDCharModifiers;
	CheckBoxWidget		m_HudDCharAgents;
	CheckBoxWidget		m_HudDCharDebug;
	CheckBoxWidget		m_HudDFreeCamCross;
	CheckBoxWidget		m_HudDVersion;
	CheckBoxWidget		m_ShowProtected;
	CheckBoxWidget		m_ShowOthers;
	CheckBoxWidget		m_ClearInventory;
	
	EditBoxWidget m_ObjectFilter;
	EditBoxWidget m_SpawnDistanceEditBox;
	TextWidget m_SelectedObjectText;
	string m_SelectedObject;
	static string m_LastSelectedObject;
	bool m_SelectedObjectIsPreset;
	TextListboxWidget m_ObjectsTextListbox;

	TextListboxWidget m_PresetsTextListbox;
	TextListboxWidget m_PresetItemsTextListbox;
	ButtonWidget m_PresetNewButton;
	ButtonWidget m_PresetDeleteButton;
	ButtonWidget m_PresetRenameButton;
	ButtonWidget m_PresetSetDefaultButton;
	ButtonWidget m_PresetAddItemtButton;
	ButtonWidget m_ItemNameCopy;
	ButtonWidget m_PresetRemoveItemButton;
	ButtonWidget m_ItemMoveUpButton;
	ButtonWidget m_ItemMoveDownButton;

	EditBoxWidget m_DrawDistanceWidget;
	EditBoxWidget m_QuantityEditBox;
	EditBoxWidget m_DamageEditBox;
	TextWidget m_ItemDamageLabel;
	TextWidget m_ItemQuantityLabel;

	ButtonWidget m_SpawnInInvButton;
	ButtonWidget m_SpawnGroundButton;
	ButtonWidget m_SpawnAsAttachment;
	ButtonWidget m_SpawnSpecial;
	ButtonWidget m_CloseConsoleButton;

	// Config Viewer
	EditBoxWidget m_ObjectConfigFilter;
	TextListboxWidget m_ConfigHierarchyTextListbox;
	TextListboxWidget m_ConfigVariablesTextListbox;
	ButtonWidget m_SelectedRowCopy;

	TextListboxWidget m_HelpTextListboxWidget;

	static const int TABS_GENERAL = 0;
	static const int TAB_ITEMS = 1;
	static const int TAB_CONFIGS = 2;
	static const int TAB_ENSCRIPT = 3;	
	static const int TAB_ENSCRIPT_SERVER = 4;
	static const int TABS_OUTPUT = 5;
	// -----------------------
	static const int TABS_COUNT = 6;

	Widget m_Tabs[TABS_COUNT];
	MapWidget m_DebugMapWidget;
	ButtonWidget m_Tab_buttons[TABS_COUNT];

	void SelectPreviousTab()
	{
		int currTab = m_selected_tab;
		currTab = (currTab - 1) % TABS_COUNT;
		if (currTab < 0)
		{
			currTab = TABS_COUNT - 1;
		}

		while(currTab != m_selected_tab)
		{
			if (m_Tab_buttons[currTab] != NULL)
			{
				SelectTab(currTab);
				return;
			}

			currTab = (currTab - 1) % TABS_COUNT;
			if (currTab < 0)
			{
				currTab = TABS_COUNT - 1;
			}
		}
	}
	
	void SelectNextTab()
	{
		int currTab = m_selected_tab;
		currTab = (currTab + 1) % TABS_COUNT;
		
		while(currTab != m_selected_tab)
		{
			if (m_Tab_buttons[currTab] != NULL)
			{
				SelectTab(currTab);	
				return;		
			}
			
			currTab = (currTab + 1) % TABS_COUNT;
		}
	}

	void SelectTab(int tab_id)
	{
		int i = 0;
		Widget tab = m_Tabs[tab_id];
		for (i = 0; i < TABS_COUNT; i++)
		{
			Widget t = m_Tabs[i];

			if( i == tab_id )
			{
				t.Show(true);
				
			}
			else
			{
				t.Show(false);
			}
		}
		string text;
		int index;
		if ( tab_id == TAB_ENSCRIPT )
		{
			m_ScriptServer = false;
			tab.Show(true);//since both buttons point to the same tab, this will keep the tab visible for either tab selectioin(it's a hack)
			index = m_EnscriptConsoleHistory.Count() - m_EnscriptHistoryRow - 1;
			if ( m_EnscriptConsoleHistory.IsValidIndex(index) )
			{
				text = m_EnscriptConsoleHistory.Get(index);
				//Print("setting text client " + text);
				m_EnfScriptEdit.SetText(text);
			}
			
		}
		else if( tab_id == TAB_ENSCRIPT_SERVER)
		{
			m_ScriptServer = true;
			index = m_EnscriptConsoleHistoryServer.Count() - m_EnscriptHistoryRowServer - 1;
			if ( m_EnscriptConsoleHistoryServer.IsValidIndex(index) )
			{
				text = m_EnscriptConsoleHistoryServer.Get(index);
				//Print("setting text server " + text);
				m_EnfScriptEdit.SetText( text );
			}
			
		}
		

		for (i = 0; i < TABS_COUNT; i++)
		{
			ButtonWidget button = m_Tab_buttons[i];
			button.SetState(i == tab_id);
		}
		
		m_selected_tab = tab_id;
		m_ConfigDebugProfile.SetTabSelected(m_selected_tab);
	}
	
	
	ref array<ref RemotePlayerStatDebug> m_PlayerDebugStats = new array<ref RemotePlayerStatDebug>;
	override void OnRPC(ParamsReadContext ctx)
	{
		ctx.Read(m_PlayerDebugStats);
	}
	
	


	PluginDeveloper m_Developer;
	PluginConfigDebugProfile m_ConfigDebugProfile;
	PluginConfigDebugProfileFixed m_ConfigDebugProfileFixed;
	PluginConfigViewer m_ModuleConfigViewer;
}
