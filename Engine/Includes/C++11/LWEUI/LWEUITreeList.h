#ifndef LWEUITREELIST_H
#define LWEUITREELIST_H
#include "LWEUI/LWEUI.h"
#include <LWCore/LWTypes.h>
#include <functional>

class LWEUITreeList;

struct LWEUITreeEvent {
	enum {
		Adding = 0,
		Removing,
		Moving
	};
	uint32_t m_EventType;
	uint32_t m_SourceID;
	uint32_t m_DestParentID;
	uint32_t m_DestPrevID;
};

struct LWEUITreeItem {
	static const uint32_t MaxNameSize = 256;

	LWEUITreeItem &SetValue(const LWText &Value, LWFont *Font, float FontScale);

	LWEUITreeItem &UpdateTextBounds(LWFont *Font, float FontScale);

	LWVector2f GetSize(float Scale, float BorderSize, float MinWidth, float MinHeight);

	LWEUIMaterial *GetMaterial(bool isOver, bool isDown, LWEUIMaterial *DefOffMaterial, LWEUIMaterial *DefOverMaterial, LWEUIMaterial *DefDownMaterial);

	LWEUITreeItem(const LWText &Value, void *UserData, LWFont *Font, float FontScale, LWEUIMaterial *OffMaterial = nullptr, LWEUIMaterial *OverMaterial = nullptr, LWEUIMaterial *DownMaterial = nullptr);

	LWEUITreeItem() = default;

	char m_Value[MaxNameSize];
	void *m_UserData = nullptr;
	LWVector2f m_TextSize;
	uint32_t m_ParentID = -1;
	uint32_t m_FirstChildID = -1;
	uint32_t m_LastChildID = -1;
	uint32_t m_PrevID = -1;
	uint32_t m_NextID = -1;
	LWEUIMaterial *m_OffMaterial = nullptr;
	LWEUIMaterial *m_OverMaterial = nullptr;
	LWEUIMaterial *m_DownMaterial = nullptr;
};

/*!< \brief callback when a tree change event wants to occur(either adding a new branch/leaf, or moving an item to a different branch/leaf, the tree does not do change automatically, instead InsertAt/RemoveAt/MoveTo functions should be called based on the event being passed. */
typedef std::function<void(LWEUITreeList &, LWEUITreeEvent &, LWEUIManager &UIManager, void *UserData)> LWEUITreeChangeCallback;

class LWEUITreeList : public LWEUI {
public:
	static const uint32_t EditIDAdd = 0;
	static const uint32_t EditIDDel = 1;
	static const uint32_t BatchSize = 64; //Size of each list expansion size.


	/*!< \brief parses a TreeList object, in addition to LWEUI attributes, LWEUITreeList also takes the following attributes:
		  OverMaterial: The default material to use when the mouse is over a list item.
		  DownMaterial: The default material to use when the mouse is pressed on a list item.
		  OffMaterial: The default material to use when the mouse is not over the list item.
		  BackgroundMaterial: The default material to use for the background of the list box.
		  Font: Named font in AssetManager to use for the text of each list item.
		  MinimumHeight: A minimum cell height for each list item.
		  LineThickness: The thickness of the line to each list item.
		  HighlightMaterial: The material to color the line highlighting where an item will be moved to when dragging items.
		  LineMaterial: The material to color the line going to each item.
		  FontMaterial: Uses the color component of the material for what color to draw the text.
		  FontScale: the scale to draw the text of each list item at.
	*/
	static LWEUITreeList *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual void Destroy(void);

	LWEUITreeList &SetItemValue(uint32_t ID, const LWText &Value);

	LWEUITreeList &SetItemValuef(uint32_t ID, const char *Fmt, ...);

	/*!< \brief inserts a new leaf to the specified location.  if ParentIdx is -1 then the leaf is added to the root of the list.  If PrevChildIdx==-1 then the item is added to the head of the children for the specified leaf, otherwise it's added after the specified index. 
		 \return the id for the item inserted.
	*/
	uint32_t InsertItemAt(const LWText &Value, void *UserData, uint32_t ParentID, uint32_t PrevID, LWAllocator &Allocator, LWEUIMaterial *OffMaterial = nullptr, LWEUIMaterial *OverMaterial = nullptr, LWEUIMaterial *DownMaterial = nullptr);
	
	/*!< \brief inserts a new leaf as the last child of the parent. */
	uint32_t InsertChildLast(const LWText &Value, void *UserData, uint32_t ParentID, LWAllocator &Allocator, LWEUIMaterial *OffMaterial = nullptr, LWEUIMaterial *OverMaterial = nullptr, LWEUIMaterial *DownMaterial = nullptr);

	/*!< \brief inserts a new leaf as the first child of the parent. */
	uint32_t InsertChildFirst(const LWText &Value, void *UserData, uint32_t ParentID, LWAllocator &Allocator, LWEUIMaterial *OffMaterial = nullptr, LWEUIMaterial *OverMaterial = nullptr, LWEUIMaterial *DownMaterial = nullptr);

	/*!< \brief removes the leaf at the specified location, and any children are also removed. This function does not remove the memory used by this item+children, call Prune if that is necessary. */
	LWEUITreeList &RemoveItemAt(uint32_t ID);

	/*!< \brief moves child to new location, with it's children in tact. */
	LWEUITreeList &MoveItemTo(uint32_t SourceID, uint32_t ParentID, uint32_t PrevID);

	/*!< \brief rebuilds the tree and condenses the list's contents, This function will change the ID's for most of the tree elements. */
	LWEUITreeList &Prune(LWAllocator &Allocator);

	/*!< \brief returns true if ChildID has SourceID as a parent somewhere in it's chain. */
	bool IsParentOf(uint32_t SourceID, uint32_t ChildID);

	LWEUITreeList &Clear(void);

	template<class Method, class Obj>
	LWEUITreeList &SetChangeCallbackMethod(Method CB, Obj *O, void *UserData) {
		return SetChangeCallback(std::bind(CB, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), UserData);
	}

	/*!< \brief if the tree change callback is set, then adding, removing, and dragging the tree is enabled, and such events are posted to the TreeChangeCallback function. */
	LWEUITreeList &SetChangeCallback(LWEUITreeChangeCallback Callback, void *UserData);

	LWEUITreeList &SetBackgroundMaterial(LWEUIMaterial *Material);

	LWEUITreeList &SetFontMaterial(LWEUIMaterial *Material);

	LWEUITreeList &SetOffMaterial(LWEUIMaterial *Material);

	LWEUITreeList &SetOverMaterial(LWEUIMaterial *Material);

	LWEUITreeList &SetDownMaterial(LWEUIMaterial *Material);

	LWEUITreeList &SetLineMaterial(LWEUIMaterial *Material);

	LWEUITreeList &SetHighlightMaterial(LWEUIMaterial *Material);

	LWEUITreeList &SetLineThickness(float LineThickness);

	LWEUITreeList &SetFont(LWFont *Font);

	LWEUITreeList &SetScroll(const LWVector2f &Scroll);

	LWEUITreeList &SetFontScale(float FontScale);

	LWEUITreeList &SetMinimumHeight(float MinHeight);

	LWEUIMaterial *GetBackgroundMaterial(void);

	LWEUIMaterial *GetFontMaterial(void);

	LWEUIMaterial *GetOffMaterial(void);

	LWEUIMaterial *GetOverMaterial(void);

	LWEUIMaterial *GetDownMaterial(void);

	LWEUIMaterial *GetLineMaterial(void);

	LWEUIMaterial *GetHighlightMaterial(void);

	LWFont *GetFont(void);

	float GetMinimumHeight(void) const;

	float GetLineThickness(void) const;

	float GetFontScale(void) const;

	LWVector2f GetScroll(void) const;

	LWVector2f GetMaxScroll(void) const;

	LWVector2f GetScrollPageSize(void) const;

	uint32_t GetFirstChildID(void) const;

	uint32_t GetLastChildID(void) const;

	uint32_t GetOverID(void) const;

	LWEUITreeItem *GetOverItem(void);

	LWEUITreeItem &GetItem(uint32_t ID);

	const LWEUITreeItem &GetItem(uint32_t ID) const;

	LWEUITreeList(const LWVector4f &Position, const LWVector4f &Size, uint64_t Flags);
private:
	LWEUITreeItem *m_List = nullptr;
	LWEUITreeItem *m_OldList = nullptr;
	LWEUIMaterial *m_BackgroundMaterial = nullptr;
	LWEUIMaterial *m_OffMaterial = nullptr;
	LWEUIMaterial *m_OverMaterial = nullptr;
	LWEUIMaterial *m_DownMaterial = nullptr;
	LWEUIMaterial *m_FontMaterial = nullptr;
	LWEUIMaterial *m_LineMaterial = nullptr;
	LWEUIMaterial *m_HighlightMaterial = nullptr;
	void *m_UserData = nullptr;
	LWEUITreeChangeCallback m_TreeChangeCallback = nullptr;
	LWEUITreeItem m_EditAddItem;
	LWEUITreeItem m_EditDelItem;
	LWVector2f m_Scroll = LWVector2f();
	LWVector2f m_MaxScroll = LWVector2f();
	LWFont *m_Font = nullptr;
	float m_MinimumHeight = 1.0f;
	float m_FontScale = 1.0f;
	float m_LineThickness = 1.0f;
	uint32_t m_FirstChildID = -1;
	uint32_t m_LastChildID = -1;
	uint32_t m_ListLength = 0;
	uint32_t m_ListBufferSize = 0;
	uint32_t m_OverID = -1;
	uint32_t m_OverEditID = -1;
	uint32_t m_DraggingID = -1;
	uint32_t m_DragDestParentID = -1;
	uint32_t m_DragDestPrevID = -1;
	LWVector2f m_DragPosition = LWVector2f(-1.0f);
};

#endif