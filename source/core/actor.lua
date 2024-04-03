import "core/cyclic-array"
import "core/history-stack"
import "core/ticker"

class('Actor').extends()

local ticker <const> = Ticker

local ACTOR_HISTORY_SIZE = 12

local nextActorId = 1

local function createDelta(stateConstructor)
  return {
    forward = {
      moveTo = { x = 0, y = 0 },
      state = stateConstructor()
    },
    backward = {
      moveTo = { x = 0, y = 0 },
      state = stateConstructor()
    }
  }
end

function Actor:init(sprite, stateConstructor)
  self.sprite = sprite
  self.id = nextActorId
  self.state = stateConstructor()
  self.undoStack = HistoryStack(ACTOR_HISTORY_SIZE)
  self.redoStack = HistoryStack(ACTOR_HISTORY_SIZE)
  self.deltaPool = CyclicArray(
    ACTOR_HISTORY_SIZE, 
    function ()
      return createDelta(stateConstructor)
    end
  )
  nextActorId += 1
end

function Actor:add() 
  self.sprite:add()
  ticker.add(self)
end

function Actor:remove()
  self.sprite:remove()
  ticker.remove(self)
end

function Actor:setupSubDelta(forwardOrBack)
  forwardOrBack.moveTo.x = self.sprite.x
  forwardOrBack.moveTo.y = self.sprite.y
  self:copyStateTo(forwardOrBack.state)
end

function Actor:act() 
  local delta = self.redoStack:pop()
  if delta then
    self:apply(delta.forward)
    self.undoStack:push(delta)
    return
  end

  delta = self.deltaPool:nextElement()
  self:setupSubDelta(delta.backward)
  self:setupSubDelta(delta.forward)
  self:plan(delta.forward)
  self:apply(delta.forward)
  self.undoStack:push(delta)
end

function Actor:undo()
  local delta = self.undoStack:pop()
  if delta then
    self:apply(delta.backward)
    self.redoStack:push(delta)
    return
  end
end

function Actor:apply(delta)
  if self.sprite.x ~= delta.moveTo.x or self.sprite.y ~= delta.moveTo.y then
    self.sprite:moveTo(delta.moveTo.x, delta.moveTo.y)
  end
  self:copyStateFrom(delta.state)
end

function Actor:copyStateTo(state) end
function Actor:copyStateFrom(state) end
function Actor:plan(stateDelta) end
