import "CoreLibs/graphics"
import "CoreLibs/object"
import "CoreLibs/sprites"
import "CoreLibs/timer"

import "actors/ant"

local ant = Ant()
ant:moveTo(200, 120)
ant:add()

function playdate.update()
  playdate.graphics.sprite.update()
  playdate.timer.updateTimers()
end
