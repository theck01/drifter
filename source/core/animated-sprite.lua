import 'core/fps-timers'

local fpsTimer = FPSTimer
local defaultOptions <const> = { frame = 1, fps = 12, runOnce = false }

class('AnimatedSprite').extends(playdate.graphics.sprite)

function AnimatedSprite:init(imgTable, options)
  AnimatedSprite.super.init(self, imgTable[1])
  self:setImageTable(imgTable, options)
end

function AnimatedSprite:setImageTable(imgTable, options) 
  local optionsToUse <const> = options or defaultOptions

  if self.timerCallbackId then
    fpsTimer.remove(self.timerCallbackId)
  end

  local newFrame <const> = optionsToUse.frame or defaultOptions.frame
  local newFPS <const> = optionsToUse.fps or defaultOptions.fps
  local newLoop <const> = not optionsToUse.runOnce

  if self.fps ~= newFPS then
    if self.timerCallbackId then
      fpsTimer.remove(self.timerCallbackId)
    end

    local thisSprite <const> = self
    self.timerCallbackId = fpsTimer.add(
      function () thisSprite:_stepAnimation() end,
      newFPS,
      not newLoop
    )
  end

  self.imgTable = imgTable
  self.frame = newFrame
  self.fps = newFPS
  self.loop = newLoop
  self:setImage(self.imgTable[self.frame])
end

function AnimatedSprite:_stepAnimation()
  if self.frame >= #self.imgTable then
    self.frame = 1
  else
    self.frame = self.frame + 1
  end
  self:setImage(self.imgTable[self.frame])
end
