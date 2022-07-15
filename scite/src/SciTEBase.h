// SciTE - Scintilla based Text Editor
/** @file SciTEBase.h
 ** Definition of platform independent base class of editor.
 **/
// Copyright 1998-2011 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SCITEBASE_H
#define SCITEBASE_H

extern const GUI::gui_char appName[];

extern const GUI::gui_char propUserFileName[];
extern const GUI::gui_char propGlobalFileName[];
extern const GUI::gui_char propAbbrevFileName[];

constexpr int IntFromTwoShorts(short a, short b) noexcept {
	return (a) | ((b) << 16);
}

#if defined (RB_SUBMENU) || defined (RB_ECM)
//!-start-[ExtendedContextMenu]
class MenuEx {
public:
	MenuEx(void* _id = 0) : mid(_id) {}
	void* GetID() const { return mid; }
	void Add(const GUI::gui_char* label = 0, int cmd = 0, int enabled = 1, const char* mnemonic = 0, int position = -1);
	void AddSubMenu(const GUI::gui_char* label, MenuEx& subMenu, int position = -1);
	void RemoveItems(int fromID = 0, int toID = -1);
	void CreatePopUp(MenuEx* parentMenu);
	void Destroy();
	void Show(GUI::Point pt, GUI::Window& w);
private:
	void* mid;
};
//!-end-[ExtendedContextMenu]
#endif // RB_SUBMENU || RB_ECM

/**
 * The order of menus on Windows - the Buffers menu may not be present
 * and there is a Help menu at the end.
 */
enum {
	menuFile = 0, menuEdit = 1, menuSearch = 2, menuView = 3,
	menuTools = 4, menuOptions = 5, menuLanguage = 6, menuBuffers = 7,
	menuHelp = 8
};

namespace SA = Scintilla;

constexpr int StyleMax = static_cast<int>(SA::StylesCommon::Max);
constexpr int StyleDefault = static_cast<int>(SA::StylesCommon::Default);

struct SelectedRange {
	SA::Position position;
	SA::Position anchor;
	SelectedRange(SA::Position position_= SA::InvalidPosition, SA::Position anchor_= SA::InvalidPosition) noexcept :
		position(position_), anchor(anchor_) {
	}
};

class RecentFile : public FilePath {
public:
	SelectedRange selection;
	SA::Line scrollPosition;
	RecentFile() {
		scrollPosition = 0;
	}
	RecentFile(const FilePath &path_, SelectedRange selection_, SA::Line scrollPosition_) :
		FilePath(path_), selection(selection_), scrollPosition(scrollPosition_) {
	}
	void Init() noexcept override {
		FilePath::Init();
		selection.position = SA::InvalidPosition;
		selection.anchor = SA::InvalidPosition;
		scrollPosition = 0;
	}
};

struct BufferState {
public:
	RecentFile file;
	std::vector<SA::Line> foldState;
	std::vector<SA::Line> bookmarks;
};

class Session {
public:
	FilePath pathActive;
	std::vector<BufferState> buffers;
};

struct FileWorker;

// Scintilla documents can only be released by calling a method on a Scintilla
// instance so store a Scintilla instance in the release functor
struct BufferDocReleaser {
	GUI::ScintillaWindow *pSci = nullptr;	// Non-owning
	void operator()(void *pDoc) noexcept;
};

using BufferDoc = std::unique_ptr<void, BufferDocReleaser>;

class Buffer {
public:
	RecentFile file;
	BufferDoc doc;
	bool isDirty;
	bool isReadOnly;
	bool failedSave;
	bool useMonoFont;
	enum class LifeState { empty, reading, readAll, opened } lifeState;
	UniMode unicodeMode;
	time_t fileModTime;
	time_t fileModLastAsk;
	time_t documentModTime;
	enum class FindMarks { none, temporary, marked, modified} findMarks;
	std::string overrideExtension;	///< User has chosen to use a particular language
	std::vector<SA::Line> foldState;
	std::vector<SA::Line> bookmarks;
	std::unique_ptr<FileWorker> pFileWorker;
	PropSetFile props;
	enum class FutureDo { none=0, finishSave=1 } futureDo;
	Buffer();

	void Init();

	void SetTimeFromFile();
	//!-start-[OpenNonExistent]
	bool DocumentNotSaved() const {
		return (isDirty || (!file.IsUntitled() && (fileModTime == 0)));
	}
	//!-end-[OpenNonExistent]

	void DocumentModified() noexcept;
	bool NeedsSave(int delayBeforeSave) const;

	void CompleteLoading() noexcept;
	void CompleteStoring();
	void AbandonAutomaticSave();

	bool ShouldNotSave() const noexcept {
		return lifeState != LifeState::opened;
	}

	void ScheduleFinishSave() noexcept;
	bool FinishSave() noexcept;

	void CancelLoad();
};

struct BackgroundActivities {
	int loaders;
	int storers;
	size_t totalWork;
	size_t totalProgress;
	GUI::gui_string fileNameLast;
};

using BufferIndex = int;
constexpr BufferIndex bufferInvalid = -1;

class BufferList {
protected:
	BufferIndex current;
	BufferIndex stackcurrent;
	std::vector<BufferIndex> stack;
public:
	std::vector<Buffer> buffers;
	BufferIndex length;
	BufferIndex lengthVisible;
	bool initialised;

	BufferList();
	BufferIndex size() const noexcept;
	void Allocate(BufferIndex maxSize);
	BufferIndex Add();
	BufferIndex GetDocumentByWorker(const FileWorker *pFileWorker) const;
	BufferIndex GetDocumentByName(const FilePath &filename, bool excludeCurrent=false);
	void RemoveInvisible(BufferIndex index);
	void RemoveCurrent();
	BufferIndex Current() const noexcept;
	Buffer *CurrentBuffer();
	const Buffer *CurrentBufferConst() const;
	void SetCurrent(BufferIndex index) noexcept;
	BufferIndex StackNext();
	BufferIndex StackPrev();
	void CommitStackSelection();
	void MoveToStackTop(BufferIndex index);
	void ShiftTo(BufferIndex indexFrom, BufferIndex indexTo);
	void Swap(BufferIndex indexA, BufferIndex indexB);
	bool SingleBuffer() const noexcept;
	BackgroundActivities CountBackgroundActivities() const;
	bool SavingInBackground() const;
	bool GetVisible(BufferIndex index) const noexcept;
	void SetVisible(BufferIndex index, bool visible);
private:
	void PopStack();
};

// class to hold user defined keyboard short cuts
class ShortcutItem {
public:
	std::string menuKey; // the keyboard short cut
	std::string menuCommand; // the menu command to be passed to "SciTEBase::MenuCommand"
};

class LanguageMenuItem {
public:
	std::string menuItem;
	std::string menuKey;
	std::string extension;
};

enum {
	heightTools = 24,
	heightToolsBig = 32,
	heightTab = 24,
	heightStatus = 20,
	statusPosWidth = 256
};

/// Warning IDs.
enum {
	warnFindWrapped = 1,
	warnNotFound,
	warnNoOtherBookmark,
	warnWrongFile,
	warnExecuteOK,
	warnExecuteKO
};

/// Codes representing the effect a line has on indentation.
enum class IndentationStatus {
	none,		// no effect on indentation
	blockStart,	// indentation block begin such as "{" or VB "function"
	blockEnd,	// indentation end indicator such as "}" or VB "end"
	keyWordStart	// Keywords that cause indentation
};

struct StyleAndWords {
	int styleNumber;
	std::string words;
	StyleAndWords() noexcept : styleNumber(0) {
	}
	bool IsEmpty() const noexcept { return words.length() == 0; }
	bool IsSingleChar() const noexcept { return words.length() == 1; }
};

struct CurrentWordHighlight {
	enum class StatesOfDelay {
		noDelay,            // No delay, and no word at the caret.
		delay,              // Delay before to highlight the word at the caret.
		delayJustEnded,     // Delay has just ended. This state allows to ignore next HighlightCurrentWord (UpdateUI and SC_UPDATE_CONTENT for setting indicators).
		delayAlreadyElapsed // Delay has already elapsed, word at the caret and occurrences are (or have to be) highlighted.
	} statesOfDelay;
	bool isEnabled;
	bool textHasChanged;
	GUI::ElapsedTime elapsedTimes;
	bool isOnlyWithSameStyle;

	CurrentWordHighlight() {
		statesOfDelay = StatesOfDelay::noDelay;
		isEnabled = false;
		textHasChanged = false;
		isOnlyWithSameStyle = false;
	}
};

class Localization : public PropSetFile, public ILocalize {
	std::string missing;
public:
	bool read;
	Localization() : PropSetFile(true), read(false) {
	}
	// Deleted so Localization objects can not be copied.
	Localization(const Localization &) = delete;
	Localization(Localization &&) = delete;
	Localization &operator=(const Localization &) = delete;
	Localization &operator=(Localization &&) = delete;
	virtual ~Localization() = default;
	GUI::gui_string Text(std::string_view sv, bool retainIfNotFound=true) const override;
	void SetMissing(const std::string &missing_) {
		missing = missing_;
	}
};

class IEditorConfig;
struct SCNotification;

struct SystemAppearance {
	bool dark;
	bool highContrast;
	bool operator==(const SystemAppearance &other) const noexcept {
		return dark == other.dark && highContrast == other.highContrast;
	}
};

// Titles appear different in menus and tabs
enum class Title { menu, tab };

class SciTEBase : public ExtensionAPI, public Searcher, public WorkerListener {
protected:
	bool needIdle;
#ifdef RB_UT
	virtual void SetToolBar() = 0;	//!-add-[user.toolbar]
#endif // RB_UT

	GUI::gui_string windowName;
	FilePath filePath;
	FilePath dirNameAtExecute;
	FilePath dirNameForExecute;

#ifdef RB_MoreRecentFiles
	static constexpr int fileStackMax = 30; //!-change-[MoreRecentFiles]
	static constexpr int fileStackMaxDefault = 10; //!-add-[MoreRecentFiles]
#else
	static constexpr int fileStackMax = 10;
#endif
	RecentFile recentFileStack[fileStackMax];
	enum { fileStackCmdID = IDM_MRUFILE, bufferCmdID = IDM_BUFFER };

	static constexpr int importMax = 50;
	FilePathSet importFiles;
	enum { importCmdID = IDM_IMPORT };
	ImportFilter filter;

	enum { indicatorMatch = static_cast<int>(SA::IndicatorNumbers::Container),
	       indicatorHighlightCurrentWord,
	       indicatorSpellingMistake,
	       indicatorSentinel
	     };

	static constexpr int markerBookmark = 1;
	static constexpr int markerFilterMatch = 2;

	ComboMemory memFiles;
	ComboMemory memDirectory;
	std::string parameterisedCommand;
	std::string abbrevInsert;

	enum { languageCmdID = IDM_LANGUAGE };
	std::vector<LanguageMenuItem> languageMenu;

	// an array of short cut items that are defined by the user in the properties file.
	std::vector<ShortcutItem> shortCutItemList;

	int codePage;
	SA::CharacterSet characterSet;
	std::string language;
	int lexLanguage;
	std::vector<std::string> monospacedList;
	std::string subStyleBases;
	StringList apis;
	std::string apisFileNames;
	std::string functionDefinition;
	BufferDocReleaser docReleaser;

	int diagnosticStyleStart;
	enum { diagnosticStyles=4};

	bool stripTrailingSpaces;
	bool ensureFinalLineEnd;
	bool ensureConsistentLineEnds;

	bool indentOpening;
	bool indentClosing;
	bool indentMaintain;
	int statementLookback;
	StyleAndWords statementIndent;
	StyleAndWords statementEnd;
	StyleAndWords blockStart;
	StyleAndWords blockEnd;
	enum class PreProc { None, Start, Middle, End, Dummy };	///< Indicate the kind of preprocessor condition line
	char preprocessorSymbol;	///< Preprocessor symbol (in C, #)
	std::map<std::string, PreProc> preprocOfString; ///< Map preprocessor keywords to positions
	/// In C, if ifdef ifndef : start, else elif : middle, endif : end.

	GUI::Window wSciTE;  ///< Contains wToolBar, wTabBar, wContent, and wStatusBar
	GUI::Window wContent;    ///< Contains wEditor and wOutput

#ifdef RB_OnSendEditor
	//!-start-[OnSendEditor]
	class ScintillaWindowEditor : public GUI::ScintillaWindow
	{
	public:
		intptr_t Call(SA::Message msg, uintptr_t wParam = 0, intptr_t lParam = 0) override;
		SciTEBase* pBase;
	};
	ScintillaWindowEditor wEditor;
	friend class ScintillaWindowEditor;
	//!-end-[OnSendEditor]
#else
	GUI::ScintillaWindow wEditor;
#endif // DEBUG

	GUI::ScintillaWindow wOutput;
	GUI::ScintillaWindow *pwFocussed;
	GUI::Window wIncrement;
	GUI::Window wToolBar;
	GUI::Window wStatusBar;
	GUI::Window wTabBar;
#ifndef RB_ECM
	GUI::Menu popup; //!-remove-[ExtendedContextMenu]
#endif
	bool tbVisible;
	bool tabVisible;
	bool tabHideOne; // Hide tab bar if one buffer is opened only
	bool tabMultiLine;
	bool sbVisible;	///< @c true if status bar is visible.
	std::string sbValue;	///< Status bar text.
	int sbNum;	///< Number of the currently displayed status bar information.
	int visHeightTools;
	int visHeightTab;
	int visHeightStatus;
	int visHeightEditor;
	int heightBar;
	// Prevent automatic load dialog appearing at the same time as
	// other dialogs as this can leads to reentry errors.
	int dialogsOnScreen;
	bool topMost;
	bool wrap;
	bool wrapOutput;
	SA::Wrap wrapStyle;
	SA::IdleStyling idleStyling;
	SA::Alpha alphaIndicator;
	bool underIndicator;
	std::string foldColour;
	std::string foldHiliteColour;
	bool openFilesHere;
	bool fullScreen;
	SystemAppearance appearance;
#ifdef RB_ToolsMax
	enum { toolMax = 300 }; //!-change-[ToolsMax]
#else
	enum { toolMax = 50 };
#endif
	Extension *extender;
	bool needReadProperties;

#ifdef RB_GMI
	bool preserveFocusOnEditor; //!-add-[GoMessageImprovement]
#endif // RB_GMI

	bool quitting;
	bool canUndo;
	bool canRedo;

	int timerMask;
	enum { timerAutoSave=1 };
	int delayBeforeAutoSave;

	int heightOutput;
	int heightOutputStartDrag;
	GUI::Point ptStartDrag;
	bool capturedMouse;
	int previousHeightOutput;
	bool firstPropertiesRead;
	bool splitVertical;	///< @c true if the split bar between editor and output is vertical.
	bool bufferedDraw;
	bool bracesCheck;
	bool bracesSloppy;
	int bracesStyle;
	int braceCount;

#ifdef RB_BTCT2
	bool callTipAutomatic; //!-add-[BetterCalltips]
#endif // RB_BTCT2

	int indentationWSVisible;
	SA::IndentView indentExamine;
	bool autoCompleteIgnoreCase;
	bool imeAutoComplete;
	bool callTipUseEscapes;
	bool callTipIgnoreCase;
	bool autoCCausedByOnlyOne;
	int autoCompleteVisibleItemCount;
	std::string calltipWordCharacters;
	std::string calltipParametersStart;
	std::string calltipParametersEnd;
	std::string calltipParametersSeparators;
	std::string calltipEndDefinition;
	std::string autoCompleteStartCharacters;
	std::string autoCompleteFillUpCharacters;
	std::string autoCompleteTypeSeparator;
	std::string wordCharacters;
	std::string whitespaceCharacters;
	SA::Position startCalltipWord;
	int currentCallTip;
	int maxCallTips;
	std::string currentCallTipWord;
	SA::Position lastPosCallTip;

	bool margin;
	int marginWidth;
	enum { marginWidthDefault = 20};

	bool foldMargin;
	int foldMarginWidth;
	enum { foldMarginWidthDefault = 14};

	bool lineNumbers;
	int lineNumbersWidth;
	enum { lineNumbersWidthDefault = 4 };
	bool lineNumbersExpand;

	bool allowMenuActions;
	int scrollOutput;
	bool returnOutputToCommand;
	JobQueue jobQueue;

	bool macrosEnabled;
	std::string currentMacro;
	bool recording;

	PropSetFile propsPlatform;
	PropSetFile propsEmbed;
	PropSetFile propsBase;
	PropSetFile propsUser;
	PropSetFile propsDirectory;
	PropSetFile propsLocal;
	PropSetFile propsDiscovered;
	PropSetFile props;

	PropSetFile propsAbbrev;

	PropSetFile propsSession;

	FilePath pathAbbreviations;

	Localization localiser;

	PropSetFile propsStatus;

	std::unique_ptr<IEditorConfig> editorConfig;

	enum { bufferMax = IDM_IMPORT - IDM_BUFFER };
	BufferList buffers;

	// Handle buffers
	void *GetDocumentAt(BufferIndex index);
	void SwitchDocumentAt(BufferIndex index, void *pdoc);
	void SaveFolds(std::vector<SA::Line> &folds);
	void RestoreFolds(const std::vector<SA::Line> &folds);
	void UpdateBuffersCurrent();
	bool IsBufferAvailable() const noexcept;
	bool CanMakeRoom(bool maySaveIfDirty = true);
	void SetDocumentAt(BufferIndex index, bool updateStack = true);
	Buffer *CurrentBuffer() {
		return buffers.CurrentBuffer();
	}
	const Buffer *CurrentBufferConst() const {
		return buffers.CurrentBufferConst();
	}
	void SetBuffersMenu();
	void BuffersMenu();
	void Next();
	void Prev();
	void NextInStack();
	void PrevInStack();
	void EndStackedTabbing();

	virtual void UpdateTabs(const std::vector<GUI::gui_string> &tabNames);
	virtual void TabInsert(int index, const GUI::gui_char *title) = 0;
	virtual void TabSelect(int index) = 0;
	virtual void RemoveAllTabs() = 0;
	void ShiftTab(BufferIndex indexFrom, BufferIndex indexTo);
	void MoveTabRight();
	void MoveTabLeft();

	virtual void ReadEmbeddedProperties();
	void ReadEnvironment();
	void ReadGlobalPropFile();
	void ReadAbbrevPropFile();
	void ReadLocalPropFile();
	void ReadDirectoryPropFile();

	virtual SystemAppearance CurrentAppearance() const noexcept;
	void CheckAppearanceChanged();
	void SetPaneFocus(bool editPane) noexcept;
	GUI::ScintillaWindow &PaneFocused();
	GUI::ScintillaWindow &PaneSource(int destination);
	intptr_t CallFocusedElseDefault(int defaultValue, SA::Message msg, uintptr_t wParam = 0, intptr_t lParam = 0);
	void CallChildren(SA::Message msg, uintptr_t wParam = 0, intptr_t lParam = 0);
	std::string GetTranslationToAbout(const char *const propname, bool retainIfNotFound = true);
	SA::Position LengthDocument();
	SA::Position GetCaretInLine();
	std::string GetLine(SA::Line line);
	std::string GetCurrentLine();
	PreProc LinePreprocessorCondition(SA::Line line);
	bool FindMatchingPreprocessorCondition(SA::Line &curLine, int direction, PreProc condEnd1, PreProc condEnd2);
	bool FindMatchingPreprocCondPosition(bool isForward, SA::Position mppcAtCaret, SA::Position &mppcMatch);
	bool FindMatchingBracePosition(bool editor, SA::Position &braceAtCaret, SA::Position &braceOpposite, bool sloppy);
	void BraceMatch(bool editor);
#ifdef RB_WRNM
	virtual void WarnUser(int warnID, const char* msg = NULL, bool isCanBeAlerted = true) = 0; //!-change-[WarningMessage]
#else
	virtual void WarnUser(int warnID) = 0;
#endif // RB_WRNM
	void SetWindowName();
	void SetFileName(const FilePath &openName, bool fixCase = true);
	FilePath FileNameExt() const {
		return filePath.Name();
	}
	void ClearDocument();
	void CreateBuffers();
	void InitialiseBuffers();
	FilePath UserFilePath(const GUI::gui_char *name);
	void LoadSessionFile(const GUI::gui_char *sessionName);
	void RestoreRecentMenu();
	void RestoreFromSession(const Session &session);
	void RestoreSession();
	void SaveSessionFile(const GUI::gui_char *sessionName);
	virtual void GetWindowPosition(int *left, int *top, int *width, int *height, int *maximize) = 0;
	void SetIndentSettings();
	void SetEol();
	void New();
	void RestoreState(const Buffer &buffer, bool restoreBookmarks);
	void Close(bool updateUI = true, bool loadingSession = false, bool makingRoomForNew = false);
	static bool Exists(const GUI::gui_char *dir, const GUI::gui_char *path, FilePath *resultPath);
	void DiscoverEOLSetting();
	void DiscoverIndentSetting();
	std::string DiscoverLanguage();
	void OpenCurrentFile(long long fileSize, bool suppressMessage, bool asynchronous);
	virtual void OpenUriList(const char *) {}
	virtual bool OpenDialog(const FilePath &directory, const GUI::gui_char *filesFilter) = 0;
	virtual bool SaveAsDialog() = 0;
	virtual void LoadSessionDialog() {}
	virtual void SaveSessionDialog() {}
	void CountLineEnds(int &linesCR, int &linesLF, int &linesCRLF);
	enum OpenFlags {
		ofNone = 0, 		// Default
		ofNoSaveIfDirty = 1, 	// Suppress check for unsaved changes
		ofForceLoad = 2,	// Reload file even if already in a buffer
		ofPreserveUndo = 4,	// Do not delete undo history
		ofQuiet = 8,		// Avoid "Could not open file" message
		ofSynchronous = 16	// Force synchronous read
	};
	void TextRead(FileWorker *pFileWorker);
	void TextWritten(FileWorker *pFileWorker);
	void UpdateProgress(Worker *pWorker);
	void PerformDeferredTasks();
	enum class OpenCompletion { synchronous, completeCurrent, completeSwitch };
	void CompleteOpen(OpenCompletion oc);
	virtual bool PreOpenCheck(const GUI::gui_char *file);
	bool Open(const FilePath &file, OpenFlags of = ofNone);
	bool OpenSelected();
	void Revert();
	FilePath SaveName(const char *ext) const;
	enum SaveFlags {
		sfNone = 0, 		// Default
		sfProgressVisible = 1, 	// Show in background save strip
		sfSynchronous = 16	// Write synchronously blocking UI
	};
	enum class SaveResult {
		completed,
		cancelled
	};
	SaveResult SaveIfUnsure(bool forceQuestion = false, SaveFlags sf = sfProgressVisible);
	SaveResult SaveIfUnsureAll();
	SaveResult SaveIfUnsureForBuilt();
	bool SaveIfNotOpen(const FilePath &destFile, bool fixCase);
	void AbandonAutomaticSave();
	virtual bool Save(SaveFlags sf = sfProgressVisible);
	void SaveAs(const GUI::gui_char *file, bool fixCase);
	virtual void SaveACopy() = 0;
	void SaveToHTML(const FilePath &saveName);
	void StripTrailingSpaces();
	void EnsureFinalNewLine();
	bool PrepareBufferForSave(const FilePath &saveName);
	bool SaveBuffer(const FilePath &saveName, SaveFlags sf);
	virtual void SaveAsHTML() = 0;
	void SaveToStreamRTF(std::ostream &os, SA::Position start = 0, SA::Position end = -1);
	void SaveToRTF(const FilePath &saveName, SA::Position start = 0, SA::Position end = -1);
	virtual void SaveAsRTF() = 0;
	void SaveToPDF(const FilePath &saveName);
	virtual void SaveAsPDF() = 0;
	void SaveToTEX(const FilePath &saveName);
	virtual void SaveAsTEX() = 0;
	void SaveToXML(const FilePath &saveName);
	virtual void SaveAsXML() = 0;
	virtual FilePath GetDefaultDirectory() = 0;
	virtual FilePath GetSciteDefaultHome() = 0;
	virtual FilePath GetSciteUserHome() = 0;
	FilePath GetDefaultPropertiesFileName();
	FilePath GetUserPropertiesFileName();
	FilePath GetDirectoryPropertiesFileName();
	FilePath GetLocalPropertiesFileName();
	FilePath GetAbbrevPropertiesFileName();
	void OpenProperties(int propsFile);
	static int GetMenuCommandAsInt(const std::string &commandName);
	virtual void Print(bool) {}
	virtual void PrintSetup() {}
	void UserStripShow(const char * /* description */) override {}
	void UserStripSet(int /* control */, const char * /* value */) override {}
	void UserStripSetList(int /* control */, const char * /* value */) override {}
	std::string UserStripValue(int /* control */) override { return std::string(); }
	virtual void ShowBackgroundProgress(const GUI::gui_string & /* explanation */, size_t /* size */, size_t /* progress */) {}
	SA::Span GetSelection();
	SelectedRange GetSelectedRange();
	void SetSelection(SA::Position anchor, SA::Position currentPos);
	std::string GetCTag();
	virtual std::string GetRangeInUIEncoding(GUI::ScintillaWindow &win, SA::Span span);
	static std::string GetLine(GUI::ScintillaWindow &win, SA::Line line);
	void RangeExtend(GUI::ScintillaWindow &wCurrent, SA::Span &range,
			 bool (SciTEBase::*ischarforsel)(char ch));
	std::string RangeExtendAndGrab(GUI::ScintillaWindow &wCurrent, SA::Span &span,
				       bool (SciTEBase::*ischarforsel)(char ch), bool stripEol = true);
	std::string SelectionExtend(bool (SciTEBase::*ischarforsel)(char ch), bool stripEol = true);
	std::string SelectionWord(bool stripEol = true);
	std::string SelectionFilename();
	void SelectionIntoProperties();
	void SelectionIntoFind(bool stripEol = true);
	enum class AddSelection { next, each };
	void SelectionAdd(AddSelection add);
	virtual std::string EncodeString(const std::string &s);
	virtual void Find() = 0;
	enum class MessageBoxChoice {
		ok,
		cancel,
		yes,
		no
	};
	typedef int MessageBoxStyle;
	enum {
		// Same as Win32 MB_*
		mbsOK = 0,
		mbsYesNo = 4,
		mbsYesNoCancel = 3,
		mbsIconQuestion = 0x20,
		mbsIconWarning = 0x30
	};
	virtual MessageBoxChoice WindowMessageBox(GUI::Window &w, const GUI::gui_string &msg, MessageBoxStyle style = mbsIconWarning) = 0;
	void FailedSaveMessageBox(const FilePath &filePathSaving);
	virtual void FindMessageBox(const std::string &msg, const std::string *findItem = nullptr) = 0;
	bool FindReplaceAdvanced() const;
	SA::Position FindInTarget(const std::string &findWhatText, SA::Span range);
	// Implement Searcher
	void SetFindText(std::string_view sFind) override;
	void SetFind(std::string_view sFind) override;
	bool FindHasText() const noexcept override;
	void SetReplace(std::string_view sReplace) override;
	void SetCaretAsStart() override;
	void MoveBack() override;
	void ScrollEditorIfNeeded() override;
	SA::Position FindNext(bool reverseDirection, bool showWarnings=true, bool allowRegExp=true) override;
	void HideMatch() override;
	virtual void FindIncrement() = 0;
	int IncrementSearchMode();
	virtual void Filter() = 0;
	virtual bool FilterShowing() { return false; }
	int FilterSearch();
	virtual void FindInFiles() = 0;
	virtual void Replace() = 0;
	void ReplaceOnce(bool showWarnings=true) override;
	intptr_t DoReplaceAll(bool inSelection); // returns number of replacements or negative value if error
	intptr_t ReplaceAll(bool inSelection) override;
	intptr_t ReplaceInBuffers();
	void SetFindInFilesOptions();
	void UIClosed() override;
	void UIHasFocus() override;
	virtual void DestroyFindReplace() = 0;
	virtual void GoLineDialog() = 0;
	virtual void AbbrevDialog() = 0;
	virtual void TabSizeDialog() = 0;
	virtual bool ParametersOpen() = 0;
	virtual void ParamGrab() = 0;
	virtual bool ParametersDialog(bool modal) = 0;
	bool HandleXml(char ch);
	static std::string FindOpenXmlTag(const char sel[], SA::Position nSize);
	void GoMatchingBrace(bool select);
	void GoMatchingPreprocCond(int direction, bool select);
	virtual void FindReplace(bool replace) = 0;
	void OutputAppendString(const char *s, SA::Position len = -1);
	virtual void OutputAppendStringSynchronised(const char *s, SA::Position len = -1);
	virtual void Execute();
	virtual void StopExecute() = 0;
	void ShowMessages(SA::Line line);
#ifdef RB_GMI
	bool GoMessage(int dir); //!-change-[GoMessageImprovement]
#else
	void GoMessage(int dir);
#endif // RB_GMI

	virtual bool StartCallTip();
	std::string GetNearestWords(const char *wordStart, size_t searchLen,
				    const char *separators, bool ignoreCase=false, bool exactLen=false);
	virtual void FillFunctionDefinition(SA::Position pos = -1);
	void ContinueCallTip();
	std::string EliminateDuplicateWords(const std::string &words);
	virtual bool StartAutoComplete();
	virtual bool StartAutoCompleteWord(bool onlyOneWord);
	virtual bool StartExpandAbbreviation();
	bool PerformInsertAbbreviation();

#ifdef RB_IA
	virtual bool InsertAbbreviation(const char* data); //!-add-[InsertAbbreviation]
#endif // RB_IA

	virtual bool StartBlockComment();
	virtual bool StartBoxComment();
	virtual bool StartStreamComment();
	std::vector<std::string> GetLinePartsInStyle(SA::Line line, const StyleAndWords &saw);
	void SetLineIndentation(SA::Line line, int indent);
	int GetLineIndentation(SA::Line line);
	SA::Position GetLineIndentPosition(SA::Line line);
	void ConvertIndentation(int tabSize, int useTabs);
	bool RangeIsAllWhitespace(SA::Position start, SA::Position end);
	IndentationStatus GetIndentState(SA::Line line);
	int IndentOfBlock(SA::Line line);
	void MaintainIndentation(char ch);
	void AutomaticIndentation(char ch);
	void CharAdded(int utf32);
	void CharAddedOutput(int ch);
	void SetTextProperties(PropSetFile &ps);
	virtual void SetFileProperties(PropSetFile &ps) = 0;
	void UpdateStatusBar(bool bUpdateSlowData) override;
	SA::Position GetLineLength(SA::Line line);
	SA::Line GetCurrentLineNumber();
	SA::Position GetCurrentColumnNumber();
	SA::Line GetCurrentScrollPosition();
	virtual void AddCommand(const std::string &cmd, const std::string &dir,
				JobSubsystem jobType, const std::string &input = "",
				int flags = 0);
	virtual void AboutDialog() = 0;
	virtual void QuitProgram() = 0;
	void CloseTab(int tab);
	void CloseAllBuffers(bool loadingSession = false);
	SaveResult SaveAllBuffers(bool alwaysYes);
	void SaveTitledBuffers();
	virtual void CopyAsRTF() {}
	virtual void CopyPath() {}
	void SetFoldWidth();
	void SetLineNumberWidth();
	void MenuCommand(int cmdID, int source = 0);
	void FoldChanged(SA::Line line, SA::FoldLevel levelNow, SA::FoldLevel levelPrev);
	void ExpandFolds(SA::Line line, bool expand, SA::FoldLevel level);
	void FoldAll();
	void ToggleFoldRecursive(SA::Line line, SA::FoldLevel level);
	void EnsureAllChildrenVisible(SA::Line line, SA::FoldLevel level);
	static void EnsureRangeVisible(GUI::ScintillaWindow &win, SA::Span range, bool enforcePolicy = true);
	void GotoLineEnsureVisible(SA::Line line);
	bool MarginClick(SA::Position position, int modifiers);
	void NewLineInOutput();
	virtual void SetStatusBarText(const char *s) = 0;
	void UpdateUI(const SCNotification *notification);
	void SetCanUndoRedo(bool canUndo_, bool canRedo_);
	void CheckCanUndoRedo();
	void Modified(const SCNotification *notification);
	virtual void Notify(SCNotification *notification);
	virtual void ShowToolBar() = 0;
	virtual void ShowTabBar() = 0;
	virtual void ShowStatusBar() = 0;
	virtual void ActivateWindow(const char *timestamp) = 0;

	void RemoveFindMarks();
	SA::FindOption SearchFlags(bool regularExpressions) const;
	void MarkAll(MarkPurpose purpose) override;
	void FilterAll(bool showMatches) override;
	void BookmarkAdd(SA::Line lineno = -1);
	void BookmarkDelete(SA::Line lineno = -1);
	bool BookmarkPresent(SA::Line lineno = -1);
	void BookmarkToggle(SA::Line lineno = -1);
	void BookmarkNext(bool forwardScan = true, bool select = false);
	void BookmarkSelectAll();
	void SetOutputVisibility(bool show);
	virtual void ShowOutputOnMainThread();
	void ToggleOutputVisible();
	virtual void SizeContentWindows() = 0;
	virtual void SizeSubWindows() = 0;

	virtual void SetMenuItem(int menuNumber, int position, int itemID,
				 const GUI::gui_char *text, const GUI::gui_char *mnemonic = nullptr) = 0;
	virtual void RedrawMenu() {}
	virtual void DestroyMenuItem(int menuNumber, int itemID) = 0;
	virtual void CheckAMenuItem(int wIDCheckItem, bool val) = 0;
	virtual void EnableAMenuItem(int wIDCheckItem, bool val) = 0;
	virtual void CheckMenusClipboard();
#ifdef RB_SE
	void CheckMenusSave(); //!-add-[SaveEnabled]
#endif
	virtual void CheckMenus();
#ifndef RB_ECM
	//!-remove-[ExtendedContextMenu]
	virtual void AddToPopUp(const char *label, int cmd = 0, bool enabled = true) = 0;
#else
	//!-start-[ExtendedContextMenu]
	bool IsMenuItemEnabled(int cmd);
	void GenerateMenu(MenuEx* subMenu, const char*& userContextItem,
		const char*& endDefinition, int& item, bool& isAdded, int parent = 0);
	//!-end-[ExtendedContextMenu]
#endif // RB_ECM

	void ContextMenu(GUI::ScintillaWindow &wSource, GUI::Point pt, GUI::Window wCmd);

#ifdef RB_SUBMENU
	//!-start-[SubMenu]
	virtual MenuEx GetToolsMenu() = 0;
	//!-end-[SubMenu]
#endif // RB_SUBMENU

	void DeleteFileStackMenu();
	void SetFileStackMenu();
	bool AddFileToBuffer(const BufferState &bufferState);
	void AddFileToStack(const RecentFile &file);
	void RemoveFileFromStack(const FilePath &file);
	RecentFile GetFilePosition();
	void DisplayAround(const RecentFile &rf);
	void StackMenu(int pos);
	void StackMenuNext();
	void StackMenuPrev();

	void RemoveToolsMenu();
	void SetMenuItemLocalised(int menuNumber, int position, int itemID,
				  const char *text, const char *mnemonic);
	bool ToolIsImmediate(int item);
	void SetToolsMenu();
	JobSubsystem SubsystemType(const char *cmd);
	void ToolsMenu(int item);

	void AssignKey(SA::Keys key, SA::KeyMod mods, int cmd);
	void ViewWhitespace(bool view);
	void SetAboutMessage(GUI::ScintillaWindow &wsci, const char *appTitle);
	void SetImportMenu();
	void ImportMenu(int pos);
	void SetLanguageMenu();
	void SetPropertiesInitial();
	GUI::gui_string LocaliseMessage(const char *s,
					const GUI::gui_char *param0 = nullptr, const GUI::gui_char *param1 = nullptr, const GUI::gui_char *param2 = nullptr);
	virtual void ReadLocalization();
	std::string GetFileNameProperty(const char *name);
	virtual void ReadPropertiesInitial();
	void ReadFontProperties();
	void SetOverrideLanguage(int cmdID);
	StyleAndWords GetStyleAndWords(const char *base);
	std::string ExtensionFileName() const;
	void SetElementColour(SA::Element element, const char *key);
	static const char *GetNextPropItem(const char *pStart, char *pPropItem, size_t maxLen) noexcept;
	void ForwardPropertyToEditor(const char *key);
	struct MarkerAppearance {
		SA::ColourAlpha fore;
		SA::ColourAlpha back;
		SA::ColourAlpha backSelected;
		int strokeWidth;
	};
	void DefineMarker(SA::MarkerOutline marker, SA::MarkerSymbol markerType, MarkerAppearance markerAppearance);
	void ReadAPI(const std::string &fileNameForExtension);
	std::string FindLanguageProperty(const char *pattern, const char *defaultValue = "");
	void SetRepresentations();
	virtual void ReadProperties();
	void ReadEditorConfig(const std::string &fileNameForExtension);
	std::string StyleString(const char *lang, int style) const;
	StyleDefinition StyleDefinitionFor(int style);
	void SetOneStyle(GUI::ScintillaWindow &win, int style, const StyleDefinition &sd);
	void SetStyleBlock(GUI::ScintillaWindow &win, const char *lang, int start, int last);
	void SetStyleFor(GUI::ScintillaWindow &win, const char *lang);
	static void SetOneIndicator(GUI::ScintillaWindow &win, int indicator, const IndicatorDefinition &ind);
	void ReloadProperties();

	void CheckReload();
	void Activate(bool activeApp);
	GUI::Rectangle GetClientRectangle();
	void Redraw();
	int NormaliseSplit(int splitPos);
	void MoveSplit(GUI::Point ptNewDrag);

	virtual void TimerStart(int mask);
	virtual void TimerEnd(int mask);
	void OnTimer();
	virtual void SetIdler(bool on);
	void OnIdle();

	void SetHomeProperties();
	void UIAvailable();
	void PerformOne(char *action);
	void StartRecordMacro();
	void StopRecordMacro();
	void StartPlayMacro();
	bool RecordMacroCommand(const SCNotification *notification);
	void ExecuteMacroCommand(const char *command);
	void AskMacroList();
	bool StartMacroList(const char *words);
	void ContinueMacroList(const char *stext);
	bool ProcessCommandLine(const GUI::gui_string &args, int phase);
	virtual bool IsStdinBlocked();
	void OpenFromStdin(bool UseOutputPane);
	void OpenFilesFromStdin();
	enum GrepFlags {
		grepNone = 0, grepWholeWord = 1, grepMatchCase = 2, grepStdOut = 4,
		grepDot = 8, grepBinary = 16, grepScroll = 32
	};
	virtual bool GrepIntoDirectory(const FilePath &directory);
#ifdef RB_FRLS
	void GrepRecursive(GrepFlags gf, const FilePath &baseDir, const char* searchString, const GUI::gui_char* fileTypes, size_t basePath); //!-change-[FindResultListStyle]
#else
	void GrepRecursive(GrepFlags gf, const FilePath &baseDir, const char *searchString, const GUI::gui_char *fileTypes);
#endif // RB_FRLS

	void InternalGrep(GrepFlags gf, const GUI::gui_char *directory, const GUI::gui_char *fileTypes,
			  const char *search, SA::Position &originalEnd);
	void EnumProperties(const char *propkind);
	void SendOneProperty(const char *kind, const char *key, const char *val);
	void PropertyFromDirector(const char *arg);
	void PropertyToDirector(const char *arg);
	// ExtensionAPI
	intptr_t Send(Pane p, SA::Message msg, uintptr_t wParam = 0, intptr_t lParam = 0) override;
	std::string Range(Pane p, SA::Span range) override;
	void Remove(Pane p, SA::Position start, SA::Position end) override;
	void Insert(Pane p, SA::Position pos, const char *s) override;
	void Trace(const char *s) override;
	std::string Property(const char *key) override;
	void SetProperty(const char *key, const char *val) override;
	void UnsetProperty(const char *key) override;
	uintptr_t GetInstance() override;
	void ShutDown() override;
	void Perform(const char *actionList) override;
	void DoMenuCommand(int cmdID) override;
	SA::ScintillaCall &PaneCaller(Pane p) noexcept override;

#ifdef RB_LFL
	std::string GetTranslation(const char* s, bool retainIfNotFound = true); //!-add-[LocalizationFromLua]
#endif // RB_LFL
#ifdef RB_PDFL
	bool ShowParametersDialog(const char* msg); //!-add-[ParametersDialogFromLua]
#endif // RB_PDFL

	// Valid CurrentWord characters
	bool iswordcharforsel(char ch);
	bool isfilenamecharforsel(char ch);
	bool islexerwordcharforsel(char ch);
#ifdef RB_OMC
	int OnMenuCommandCallsCount; //!-add-[OnMenuCommand]
#endif // RB_OMC

	CurrentWordHighlight currentWordHighlight;
	void HighlightCurrentWord(bool highlight);
	MatchMarker matchMarker;
	MatchMarker findMarker;
public:

	enum { maxParam = 4 };

	explicit SciTEBase(Extension *ext = 0);
	// Deleted copy-constructor and assignment operator.
	SciTEBase(const SciTEBase &) = delete;
	SciTEBase(SciTEBase &&) = delete;
	void operator=(const SciTEBase &) = delete;
	void operator=(SciTEBase &&) = delete;
	~SciTEBase() override;

	void Finalise();

	GUI::WindowID GetID() const noexcept { return wSciTE.GetID(); }
#ifdef RB_GAP
	//!-start-[GetApplicationProps]
	static SciTEBase* GetApplicationInstance();
	static PropSetFile* GetProps() {
		SciTEBase* app = GetApplicationInstance();
		if (app != NULL) return &(app->props);
		return NULL;
	}
	//!-end-[GetApplicationProps]
#endif
	bool PerformOnNewThread(Worker *pWorker);
	// WorkerListener
	void PostOnMainThread(int cmd, Worker *pWorker) override = 0;
	virtual void WorkerCommand(int cmd, Worker *pWorker);
};

int ControlIDOfCommand(unsigned long) noexcept;
SA::Colour ColourOfProperty(const PropSetFile &props, const char *key, SA::Colour colourDefault);
SA::ColourAlpha ColourAlphaOfProperty(const PropSetFile &props, const char *key, SA::ColourAlpha colourDefault);
void WindowSetFocus(GUI::ScintillaWindow &w);

// Test if an enum class value has the bit flag(s) of test set.
template <typename T>
constexpr bool FlagIsSet(T value, T test) {
	return (static_cast<int>(value) & static_cast<int>(test)) == static_cast<int>(test);
}

#endif