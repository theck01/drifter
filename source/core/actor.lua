import "core/ticker"

class('Actor').extends()

local nextActorId = 1

function Actor:init(sprite)
  self.sprite = sprite
  self.id = nextActorId
  nextActorId += 1
end

function Actor:add() 
  self.sprite:add()
  Ticker.add(self)
end

function Actor:remove()
  self.sprite:remove()
  Ticker.remove(self)
end

function Actor:act() end
