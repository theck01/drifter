import "CoreLibs/crank"
import "CoreLibs/graphics"
import "CoreLibs/object"
import "CoreLibs/sprites"
import "CoreLibs/timer"
import "CoreLibs/ui"

import "actors/ant"
import "core/ticker"

local ant = Ant()
ant.sprite:moveTo(200, 120)
ant:add()

function playdate.update()
  playdate.graphics.sprite.update()
  playdate.timer.updateTimers()

  if playdate.isCrankDocked() then
    playdate.ui.crankIndicator:draw()
  end

  playdate.drawFPS(0, 0)
end

if playdate.isSimulator then
  local isPaused = false
  local simulatedCrank = playdate.timer.new(
    1000 / 30,
    function ()
      Ticker._simulateTick()
    end
  )
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
