-- SPDX-License-Identifier: GPL-3.0-or-later

--- Skill用来描述一个技能。
---
---@class Skill : Object
---@field public name string @ 技能名
---@field public trueName string @ 技能真名
---@field public package Package @ 技能所属的包
---@field public frequency Frequency @ 技能发动的频繁程度，通常compulsory（锁定技）及limited（限定技）用的多。
---@field public visible boolean @ 技能是否会显示在游戏中
---@field public mute boolean @ 决定是否关闭技能配音
---@field public anim_type string @ 技能类型定义
---@field public related_skills Skill[] @ 和本技能相关的其他技能，有时候一个技能实际上是通过好几个技能拼接而实现的。
---@field public attached_equip string @ 属于什么装备的技能？
---@field public switchSkillName string
local Skill = class("Skill")

---@alias Frequency integer

Skill.Frequent = 1
Skill.NotFrequent = 2
Skill.Compulsory = 3
Skill.Limited = 4
Skill.Wake = 5
Skill.Quest = 6

--- 构造函数，不可随意调用。
---@param name string @ 技能名
---@param frequency Frequency @ 技能发动的频繁程度，通常compulsory（锁定技）及limited（限定技）用的多。
function Skill:initialize(name, frequency)
  -- TODO: visible, lord, etc
  self.name = name
  -- skill's package is assigned when calling General:addSkill
  -- if you need skills that not belongs to any general (like 'jixi')
  -- then you should use general function addRelatedSkill to assign them
  self.package = { extensionName = "standard" }
  self.frequency = frequency
  self.visible = true
  self.lordSkill = false
  self.mute = false
  self.anim_type = ""
  self.related_skills = {}
  self.attachedKingdom = {}

  local name_splited = name:split("__")
  self.trueName = name_splited[#name_splited]

  if string.sub(name, 1, 1) == "#" then
    self.visible = false
  end
  if string.sub(name, #name) == "$" then
    self.name = string.sub(name, 1, #name - 1)
    self.lordSkill = true
  end

  self.attached_equip = nil
end

function Skill:__tostring()
  return "<Skill " .. self.name .. ">"
end

--- 为一个技能增加相关技能。
---@param skill Skill @ 技能
function Skill:addRelatedSkill(skill)
  table.insert(self.related_skills, skill)
end

--- 确认本技能是否为装备技能。
---@return boolean
function Skill:isEquipmentSkill()
  return self.attached_equip and type(self.attached_equip) == 'string' and self.attached_equip ~= ""
end

--- 判断技能是不是对于某玩家而言失效了。
---
--- 它影响的是hasSkill，但也可以单独拿出来判断。
---@param player Player @ 玩家
---@return boolean
function Skill:isEffectable(player)
  local nullifySkills = Fk:currentRoom().status_skills[InvaliditySkill] or {}
  for _, nullifySkill in ipairs(nullifySkills) do
    if self.name ~= nullifySkill.name and nullifySkill:getInvalidity(player, self) then
      return false
    end
  end

  return true
end

function Skill:addAttachedKingdom(kingdom)
  table.insertIfNeed(self.attachedKingdom, kingdom)
end

function Skill:isSwitchSkill()
  return self.switchSkillName and type(self.switchSkillName) == 'string' and self.switchSkillName ~= ""
end

return Skill
