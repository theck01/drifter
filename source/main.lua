import "CoreLibs/crank"
import "CoreLibs/graphics"
import "CoreLibs/object"
import "CoreLibs/sprites"
import "CoreLibs/timer"
import "CoreLibs/ui"

import "actors/ant"

local ant = Ant()
ant.sprite:moveTo(200, 120)
ant:add()

function playdate.update()
  playdate.graphics.sprite.update()
  playdate.timer.updateTimers()

  if playdate.isCrankDocked() then
    playdate.ui.crankIndicator:draw()
  end
end
