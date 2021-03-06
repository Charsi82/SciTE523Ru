//  Add this block for run only with 32bit WScript|CScript
//  ??????? ??? ????, ????? ?????? ?????????? ?????? ? 32bit ?????? ??????????????? WScript|CScript
//  ===============================================
var WshShell = new ActiveXObject("WScript.Shell");

if ((WshShell.ExpandEnvironmentStrings("%ProgramFiles(x86)%") != "%ProgramFiles(x86)%") && (WScript.FullName.indexOf(WshShell.ExpandEnvironmentStrings("%WINDIR%\\SysWOW64")) != 0)) {
	x = WScript.FullName.replace(/^.+\\/,'');
	cmd = WshShell.ExpandEnvironmentStrings('%WINDIR%\\SysWOW64\\') + x + ' "' + WScript.ScriptFullName + '"';
	WshShell.Run(cmd);
	WScript.Quit();
}

//  EXAMPLES AND DESCRIPTIONS OF ALL AVAILABLE METHODS SCITE HELPER
//  ??????? ? ???????? ???? ????????? ??????? SCITE HELPER
//  ===============================================
//  In the beginning we create object
//  ??????? ??????? ??????
try {
	var SciTE=new ActiveXObject("SciTE.Helper");
} catch(e) {
	WScript.Echo("Please install SciTE Helper before!");
	WScript.Quit(1);
}

// ?????? ?????? ? ???? ??????? (??? ???????? ? ???????? ??????)
// Writes string to the output pane (no prefix, no newlines)
SciTE.Trace("Example of all available methods:\\n\\n");

// SciTE window size and position
SciTE.Trace ("position.Left = " + SciTE.Left + "\\n");
SciTE.Trace ("position.top = " + SciTE.Top + "\\n");
SciTE.Trace ("position.width = " + SciTE.Width + "\\n");
SciTE.Trace ("position.height = " + SciTE.Height + "\\n\\n");

//  Get all text with active page
//  ????????? ???? ????? ? ???????? ????????
var all_text = SciTE.GetText;

SciTE.Send ("find:scite");
//  Get only selected text with active page
//  ????????? ?????? ?????????? ????? ? ???????? ????????
var sel_text = SciTE.GetSelText;
SciTE.Trace ("Selected text: \"" + sel_text + "\"\\n");

//  Replace selected on active page text on our
//  ???????? ?????????? ?? ???????? ???????? ????? ?? ???
//~ SciTE.ReplaceSel ("<http://scite-ru.org>");

//  Run command use SciTE Lua Scripting Extension
//  ????????? LUA ??????? ? ???????? ?????????
var CurrentPos = SciTE.LUA("editor.CurrentPos");
SciTE.Trace ("editor.CurrentPos = " + CurrentPos + "\\n");

//  ?????? ???? ? property ? ??? ????????
//  Set the value of a property
var value = WScript.FullName;
SciTE.Props("my.key") = WScript.Name;

//  ?????? ???????? ????????? ?????
//  Return the value of a property
var prop = SciTE.Props ("my.key");
SciTE.Trace ("my.key = " + prop + "\\n");

//  Send actions use SciTE Director Interface
//  List of all available commands - in file SciTEDirector.html
//  ???????? ??????? ????????? SciTE Director Interface
//  ?????? ???? ????????? ?????? - ? ????? SciTEDirector.html
var filename  = SciTE.Send ("askfilename:");
filename = filename.replace(/\\/g,"\\\\");
SciTE.Trace (filename + "\\n");

//  Run internal menu command SciTE (call "About" window)
//  List of all available commands - in file SciTE.h
//  ???????? ?????????? ??????? ???? SciTE (?????? "? ?????????")
//  ?????? ???? ????????? ?????? - ? ????? SciTE.h
SciTE.MenuCommand (902);

//  ?? ??? ?? ??? ????? :)
SciTE.About();
