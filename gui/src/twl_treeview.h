#pragma once
class TTreeView : public TNotifyWin {
	SelectionHandler       m_on_select;
	NotifyEventHandler m_on_selection_changing;
	TEventWindow* m_form;
	bool m_has_images;
public:

	void on_select(SelectionHandler handler)
	{
		m_on_select = handler;
	}

	void on_selection_changing(NEH handler)
	{
		m_on_selection_changing = handler;
	}

	TTreeView(TEventWindow* form, bool has_lines = true, bool editable = false);
	Handle add(Handle parent, pchar caption, int idx1 = 0, int idx2 = -1, bool has_children = false, void* data = 0);
	TWin* get_parent_win() { return m_form; };
	void* get_data(Handle pn);
	int add_item(pchar text, int idx = 0, void* data = NULL);
	void select(Handle p);
	void set_image_list(TImageList* il, bool normal = true);
	// override
	int handle_notify(void* p);
};
