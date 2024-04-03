import "CoreLibs/crank"
import "CoreLibs/graphics"
import "CoreLibs/object"
import "CoreLibs/sprites"
import "CoreLibs/timer"
import "CoreLibs/ui"

import "actors/ant"
import "core/ticker"

local kButtonB <const> = playdate.kButtonB
local sprite <const> = playdate.graphics.sprite
local timer <const> = playdate.timer
local ui <const> = playdate.ui
local buttonIsPressed <const> = playdate.buttonIsPressed
local isCrankDocked <const> = playdate.isCrankDocked
local drawFPS <const> = playdate.drawFPS

local ticker <const> = Ticker

for i=1,100 do
  local ant = Ant()
  ant.sprite:moveTo(math.random(60,340), math.random(20,220))
  ant:add()
end

function playdate.update()
  sprite.update()
  timer.updateTimers()

  if buttonIsPressed(kButtonB) then
    ticker._simulateReverseTick()
  end

  if isCrankDocked() then
    ui.crankIndicator:draw()
  end

  drawFPS(0, 0)
end

if playdate.isSimulator then
  local isPaused = false
  local simulatedCrank = timer.new(1000 / TICKS_PER_ROTATION, ticker._simulateTick)
  simulatedCrank.repeats = true

  function playdate.keyPressed(key)
    if key == 'n' and not isPaused then
      simulatedCrank:pause()
      isPaused = true
    elseif key == 'y' and isPaused then
      simulatedCrank:start()
      isPaused = false
    end
  end
end
