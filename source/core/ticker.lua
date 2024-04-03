class('Ticker').extends()

TICKS_PER_ROTATION = 24 

local tickerSingleton = nil

function Ticker.add(actor)
  tickerSingleton:_add(actor)
end

function Ticker.remove(actor)
  tickerSingleton:_remove(actor)
end

function Ticker._simulateTick()
  tickerSingleton:_processTick() 
end

function Ticker:init()
  self.actors = {}
end

function Ticker:_add(actor)
  for i=1, #self.actors do
    if self.actors[i].id == actor.id then
      error('Cannot add actor id:' .. actor.id .. ' to ticker, already present')
    end
  end
  table.insert(self.actors, actor)
end

function Ticker:_remove(actor)
  for i=1, #self.actors do
    if self.actors[i].id == actor then
      table.remove(self.actors, i)
      return
    end
  end
  error('Cannot remove actor id:' .. actor.id .. ' from ticker, not present')
end

function Ticker:_processTick()
  for i=1, #self.actors do
    self.actors[i]:act()
  end
end

local getCrankTicks <const> = playdate.getCrankTicks
function playdate.cranked()
  local ticks = getCrankTicks(TICKS_PER_ROTATION)
  for i=1, ticks do
    tickerSingleton:_processTick()
  end
end

tickerSingleton = Ticker()
