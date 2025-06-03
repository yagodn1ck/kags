--|============================|
--|         OpenAdmins.        |
--|       Автор: SkyDrive_     |
--| Проект McSkill, cервер HTC |
--|         14.05.2017         |
--|        Version: 1.02       |
--|============================|
local component = require("component")
local computer=require("computer")
local event = require("event")
local term = require("term")
local shell = require("shell")
local fs = require("filesystem")
local unicode=require("unicode")
local serial = require("serialization")
if not fs.exists("/lib/Sky.lua") then
  shell.execute("wget http://pastebin.com/raw/uL180xek /lib/Sky.lua")
end
local sky = require("Sky")
local g = component.gpu
event.shouldInterrupt = function () return false end
--------------------Настройки--------------------
local WIDTH, HEIGHT = 146, 42 --Разрешение моника 146/112 x 42
local COLOR1 = 0x00ffff --Рамка
local COLOR2 = 0x333333 --Цвет кнопок
local UPDATE = 300 --Апдейт отображения информации в сек.
local SCOREBOARDS = {"Admin", "TAdmin", "Global", "GD", "GM", "STMod", "Dev", "Builder", "Modn",
"Helper2", "Helper1"} --Названия Скорбордов (Создать такие же на серве, либо поменять тут)
local MOD_CHAT_COLOR = {"&8", "&8", "&8",  "&9", "&9", "&3", "&5", "&5", "&6", "&2", "&a"} --Изменение цвета ника в Мод.Чате
-------------------------------------------------
 
local mid = WIDTH / 2
local sel = nil
local admins
local timer = 0
local stat = {"&0[&4Админ&0] - &0", "&0[&4Тех.Админ&0] - &0", "&0[&4Глобал Мод&0] - &0", "&0[&9Дизайнер&0] - &3","&0[&9Гл.Модератор&0] - &3",
"&0[&3Ст.Модератор&0] - &3", "&0[&5Разработчик&0] - &d", "&0[&5Строитель&0] - &d", "&0[&4Модератор&0] - &6","&0[&2Помощник&0] - &2", "&0[&aСтажёр&0] - &2"}
 
file = io.open(shell.getWorkingDirectory() .. "/AdminsBD.lua", "r")
local reads = file:read(9999999)
if reads ~= nil then
  admins = serial.unserialize("{" .. reads .. "}")
else
  admins = {}
end
file:close()
 
g.setResolution(WIDTH, HEIGHT)
sky.Ram("OpenAdmins", COLOR1, COLOR2, WIDTH, HEIGHT)
 
function isOnline(nick)
result,reason=computer.addUser(nick)
computer.removeUser(nick)
if result==true then
 return "&aонлайн"
else
 return "&4оффлайн"
end
end
 
function Save()
  file = io.open(shell.getWorkingDirectory() .. "/AdminsBD.lua", "w")
  local text = ""
  for i = 1, #admins do
    text = text .. "{'"..admins[i][1].."','"..admins[i][2].."','"..admins[i][3].."'},\n"    
  end
  file:write(text)
  file:close()
end
 
function Seen(nick)
  local  status=isOnline(nick)
  return status, 0, 0, 0, 0, 0
end
 
function Update()
  for i = 1, #admins do
    g.set(mid-45,i+13,"                                     ")
    sky.Text(mid-45, i+13, admins[i][1] .. admins[i][2])
    sky.Text(mid-8, i+13, admins[i][3] .. "  ")
 
    sky.Text(mid+4, i+13, "&6" .. sky.playtime(admins[i][2]) .. "    ")
 
    local status, year, month, day, hour, minute = Seen(admins[i][2]) 
    g.set(mid+20,i+13,"                       ")
    if status == "&4error" then
      sky.Text(mid+20,i+13, status)
    elseif year ~= 0 then
      sky.Text(mid+20,i+13, status)
    elseif month ~= 0 then
      sky.Text(mid+20,i+13, status)
    elseif day ~= 0 then
      sky.Text(mid+20,i+13, status)
    else
      sky.Text(mid+20,i+13, status)
    end
  end
end
 
function Sort()
  local buffer = {}
  for i = 1, #stat do
    for j = 1, #admins do
      if stat[i] == admins[j][1] then
        table.insert(buffer, admins[j])
      end
    end
  end
  admins = buffer
end
 
function Sel()
  for i = 1, #admins do
    sky.text(mid-51,i+13, "   ")
    sky.text(mid+45,i+13, "   ")
    if sel~= nil then
      if sel == admins[i][2] then
        sky.Text(mid-51,i+13, "&b>>>")
        sky.Text(mid+45,i+13, "&b<<<")
      end
    end
  end
end
 
function Draw()
  if sel ~= nil then
    sky.Button(mid-44,HEIGHT-4,20,3,COLOR1,COLOR2,"Повысить")
    sky.Button(mid-22,HEIGHT-4,20,3,COLOR1,COLOR2,"Понизить")
    sky.Button(mid+6,HEIGHT-4,20,3,COLOR1,COLOR2,"Male/Female")
    sky.Button(mid+28,HEIGHT-4,20,3,COLOR1,COLOR2,"Удалить")
  else
    g.fill(3,HEIGHT-6, WIDTH-11, 5, " ")
  end
end
 
function Index(array, value)
  for i = 1, #array do
    if value == array[i] then
      return i
    end
  end
end
 
function Click(w,h)
  if sel~= nil then
    local sel_id
    for i = 1, #admins do
      if sel == admins[i][2] then
        sel_id = i
        break
      end
    end
    if w>=mid-44 and w<=mid-25 and h>=HEIGHT-4 and h<=HEIGHT-2 then
      index = Index(stat,admins[sel_id][1]) - 1
      if index == 0 then index = 1 end
      admins[sel_id][1] = stat[index]   
      Sort()
    elseif w>=mid-22 and w<=mid-3 and h>=HEIGHT-4 and h<=HEIGHT-2 then
      index = Index(stat,admins[sel_id][1]) + 1
      if index == #stat + 1 then index = #stat end
      admins[sel_id][1] = stat[index] 
      Sort()
    elseif w>=mid+6 and w<=mid+25 and h>=HEIGHT-4 and h<=HEIGHT-2 then
      if admins[sel_id][3] == "&bMale" then
        admins[sel_id][3] = "&dFemale"
      else
        admins[sel_id][3] = "&bMale"
      end
    elseif w>=mid+28 and w<=mid+47 and h>=HEIGHT-4 and h<=HEIGHT-2 then
      table.remove(admins, sel_id)
      g.fill(3,13,WIDTH-4,25, " ")
    end
    Update()
    Save()
  end
 
  if w>=WIDTH-8 and w<=WIDTH-4 and h>=HEIGHT-4 and h<=HEIGHT-2 then
    sky.Text(mid-44, HEIGHT-6,"&bВведите ник:")
    term.setCursor(mid-30, HEIGHT-6)
    local n = io.read()
    table.insert(admins, {stat[#stat], n, "&bMale"})
    g.set(mid-44, HEIGHT-6,"                                                                                        ")
    Update()
    Save()
  end
 
  if h-13 >= 1 and h-13 <= #admins then
    sel = admins[h-13][2]
    Sel()
  else
    sel = nil
    Sel()
  end
 
  Draw()
end
 
sky.Button(WIDTH - 8,HEIGHT-4,5,3,COLOR1,COLOR2,"+")
Update()
 
while true do
  local e,_,w,h,_,nick = event.pull(UPDATE, "touch")
  if e == "touch" then
    Click(w,h)
  end
  if sel == nil then
    Update()
  end
end
