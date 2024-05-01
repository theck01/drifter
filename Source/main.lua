import "CoreLibs/sprites"
import "CoreLibs/ui"

local sprite <const> = playdate.graphics.sprite
local ui <const> = playdate.ui
local isCrankDocked <const> = playdate.isCrankDocked
local drawFPS <const> = playdate.drawFPS

local engineUpdate = cupdate

function playdate.update()
  engineUpdate();
  sprite.update()

  if isCrankDocked() then
    ui.crankIndicator:draw()
  end

  drawFPS(0, 0)
end
