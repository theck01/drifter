import "core/actor"
import "core/animated-sprite"

class('Ant').extends(Actor)


local ORIENTATION <const> = { LEFT = 'LEFT', RIGHT = 'RIGHT' }
local ACTION <const> = { IDLE = 'IDLE', WALK = 'WALK' }
local MAX_SPEED_PX <const> = 8
local INC_SPEED_TABLES = { [1] = 2, [2] = 4, [4] = 8, [8] = 8 }
local DEC_SPEED_TABLES = { [1] = 1, [2] = 1, [4] = 2, [8] = 4 }

local ANIMATIONS <const> = {
  IDLE = {
    RIGHT = playdate.graphics.imagetable.new('img/ant-idle-right'),
    LEFT = playdate.graphics.imagetable.new('img/ant-idle-left'),
  },
  WALK = {
    RIGHT= playdate.graphics.imagetable.new('img/ant-walk-right'),
    LEFT = playdate.graphics.imagetable.new('img/ant-walk-left'),
  }
}

local function createState()
  return {
    action = ACTION.IDLE,
    orientation = ORIENTATION.RIGHT,
    speed = 0,
    ticksBeforeNextDecision = math.random(12, 36)
  }
end

function Ant:init() 
  Ant.super.init(self, AnimatedSprite(ANIMATIONS.IDLE.RIGHT), createState)
end

function Ant:copyStateTo(state)
  state.orientation = self.state.orientation
  state.action = self.state.action
  state.speed = self.state.speed
  state.ticksBeforeNextDecision = self.state.ticksBeforeNextDecision
end

function Ant:copyStateFrom(state)
  if 
    state.orientation ~= self.state.orientation or 
    state.action ~= self.state.action 
  then
    self.sprite:setImageTable(
      ANIMATIONS[state.action][state.orientation], 
      { frame = state.action == self.state.action and self.frame or 1 }
     )
  end
  self.state.orientation = state.orientation
  self.state.action = state.action
  self.state.speed = state.speed
  self.state.ticksBeforeNextDecision = state.ticksBeforeNextDecision
end

function Ant:plan(delta)
  local makeNewDecision = false
  if self.state.action == ACTION.WALK then
    local velocity <const> = self.state.orientation == ORIENTATION.RIGHT and self.state.speed or -1 * self.state.speed
    local desiredX = self.sprite.x + velocity
    local clampedX = math.min(350, math.max(50, desiredX))
    delta.moveTo.x = clampedX
    makeNewDecision = desiredX ~= clampedX

    -- Accelerate or decelerate
    if self.state.ticksBeforeNextDecision <= 4 then 
      delta.state.speed = DEC_SPEED_TABLES[self.state.speed]
    elseif self.state.speed < MAX_SPEED_PX then
      delta.state.speed = INC_SPEED_TABLES[self.state.speed]
    end

    if delta.state.speed == nil then
      print('Error incoming')
      printTable(self.state)
    end
  end

  if self.state.ticksBeforeNextDecision == 1 then
    delta.state.ticksBeforeNextDecision = math.random(12, 36)
    makeNewDecision = true
  else
    delta.state.ticksBeforeNextDecision -= 1
  end

  if makeNewDecision then
    delta.state.action = math.random() > 0.5 and 
      ACTION.WALK or ACTION.IDLE
    if delta.state.action == ACTION.WALK then
      delta.state.orientation = self.sprite.x < 200 and 
        ORIENTATION.RIGHT or ORIENTATION.LEFT
      delta.state.speed = 1
    else
      delta.state.orientation = math.random() > 0.5 and
        ORIENTATION.RIGHT or ORIENTATION.LEFT
      delta.state.speed = 0
    end
  end
end
