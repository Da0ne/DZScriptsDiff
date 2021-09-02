class WarningMenuBase extends UIScriptedMenu
{
	void WarningMenuBase()
	{
		GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);
	}

	void ~WarningMenuBase()
	{
		if ( GetGame() && GetGame().GetMission() )
			GetGame().GetMission().PlayerControlEnable(true);
	}
	
	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("gui/layouts/day_z_dropped_items.layout");
		string text = GetText();
		if(text)
		{
			MultilineTextWidget w = MultilineTextWidget.Cast(layoutRoot.FindAnyWidget("Text"));
			if(w)
				w.SetText(text);
		}
		return layoutRoot;
	}
	
	string GetText()
	{
		return "";
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		
		if (w.GetUserID() == IDC_OK)
		{
			Close();
			return true;
		}

		return false;
	}
}

class ItemDropWarningMenu extends WarningMenuBase
{
	override string GetText()
	{
		return "#str_item_drop_notification";
	}
}

class PlayerRepositionWarningMenu extends WarningMenuBase
{
	override string GetText()
	{
		return "#str_position_change_notification";
	}
}