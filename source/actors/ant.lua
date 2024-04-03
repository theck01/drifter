import "core/animated-sprite"

class('Ant').extends(AnimatedSprite)

local ORIENTATION <const> = { LEFT = 'LEFT', RIGHT = 'RIGHT' }
local ACTION <const> = { IDLE = 'IDLE', WALK = 'WALK' }
local MAX_SPEED_PX <const> = 4
local INC_SPEED_TABLES = { [1] = 2, [2] = 4 }
local DEC_SPEED_TABLES = { [1] = 0, [2] = 1, [4] = 2 }

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

function Ant:init() 
  Ant.super.init(self, ANIMATIONS.WALK.RIGHT)
  self.state = {
    orientation = ORIENTATION.RIGHT,
    action = ACTION.WALK,
    speed = 1,
    framesBeforeNextAction = math.random(30, 90)
  }
end

function Ant:update()
  self:act()
end

function Ant:act()
  if self.state.action == ACTION.WALK then
    local velocity <const> = self.state.orientation == ORIENTATION.RIGHT and self.state.speed or -1 * self.state.speed
    local desiredX = self.x + velocity
    local clampedX = math.min(350, math.max(50, desiredX))
    self:moveTo(clampedX, self.y)

    -- If reached the end of the screen, pick another action
    if  desiredX ~= clampedX then
      self:pickNextAction()
      return
    end


    -- Accelerate or decelerate
    if self.state.framesBeforeNextAction <= 4 then 
      self.state.speed = DEC_SPEED_TABLES[self.state.speed]
    elseif self.state.speed < MAX_SPEED_PX then
      self.state.speed = INC_SPEED_TABLES[self.state.speed]
    end
  end

  self.state.framesBeforeNextAction -= 1
  
  if self.state.framesBeforeNextAction <= 0 then
    self:pickNextAction()
  end
end

function Ant:pickNextAction()
  local nextAction <const> = math.random() > 0.5 and 
    ACTION.WALK or ACTION.IDLE
  local nextOrientation = nil
  if nextAction == ACTION.WALK then
    nextOrientation = self.x < 200 and ORIENTATION.RIGHT or ORIENTATION.LEFT
  else
    nextOrientation = math.random() > 0.5 and
      ORIENTATION.RIGHT or ORIENTATION.LEFT
  end

  self:setImageTable(
    ANIMATIONS[nextAction][nextOrientation], 
    { frame = nextAction == self.state.action and self.frame or 1 }
  )

  self.state = {
    orientation = nextOrientation,
    action = nextAction,
    speed = nextAction == ACTION.WALK and 1 or 0,
    framesBeforeNextAction = math.random(30, 90)
  }
end
