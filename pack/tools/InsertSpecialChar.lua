--[[--------------------------------------------------
InsertSpecialChar.lua
Authors: mozers™
Version: 0.5
------------------------------------------------------
Вставка спецсимволов (©,®,§,±,…) из раскрывающегося списка (для HTML вставляются их обозначения)
--]]--------------------------------------------------
local cp = 1

local char2html = {
	' ', '  ', '&nbsp;',
	'&', '& ', '&amp;',
	'"', '" ', '&quot;',
	'<', '< ', '&lt;',
	'>', '> ', '&gt;',
	'‘', 'вЂ', '&lsquo;',
	'’', 'вЂ™', '&rsquo;',
	'“', 'вЂњ', '&ldquo;',
	'”', 'вЂќ', '&rdquo;',
	'‹', 'вЂ№', '&lsaquo;',
	'›', 'вЂє', '&rsaquo;',
	'«', 'В«', '&laquo;',
	'»', 'В»', '&raquo;',
	'„', 'вЂћ', '&bdquo;',
	'‚', 'вЂљ', '&sbquo;',
	'·', 'В·', '&middot;',
	'…', 'вЂ¦', '&hellip;',
	'§', 'В§', '&sect;',
	'©', 'В©', '&copy;',
	'®', 'В®', '&reg;',
	'™', 'в„ў', '&trade;',
	'¦', 'В¦', '&brvbar;',
	'†', 'вЂ ', '&dagger;',
	'‡', 'вЂЎ', '&Dagger;',
	'¬', 'В¬', '&not;',
	'­', 'В­', '&shy;',
	'±', 'В±', '&plusmn;',
	'µ', 'Вµ', '&micro;',
	'‰', 'вЂ°', '&permil;',
	'°', 'В°', '&deg;',
	'€', 'в‚¬', '&euro;',
	'¤', 'В¤', '&curren;',
	'•', 'вЂў', '&bull;',
}

local function f_char2html (char)
	for index, v in ipairs(char2html) do 
		if v == char then
			if index>5 then cp = cp - 1 end -- after '>' take item from next column 
			return char2html[index + 3 - cp]
		end
	end
	return char
end

local function InsertSpecialChar(sel_value)
	if editor.Lexer == SCLEX_HTML then
		sel_value = f_char2html(sel_value)
	end
	editor:ReplaceSel(sel_value)
end

function SpecialChar()
	cp = (editor.CodePage==0) and 1 or 2
	local sep = ';'
	local n = #char2html
	local tt = {}
	for i = cp, n-3, 3 do
		table.insert(tt, char2html[i])
	end
	table.insert(tt,char2html[n - 3 + cp])
	local user_list =  table.concat(tt, sep)
	editor.AutoCSeparator = string.byte(sep)
	editor:UserListShow(12,user_list:from_utf8(0))
	editor.AutoCSeparator = string.byte(' ')
end

-- Добавляем свой обработчик события OnUserListSelection
AddEventHandler("OnUserListSelection", function(tp, sel_value)
	if tp == 12 then
		InsertSpecialChar(sel_value)
	end
end)
