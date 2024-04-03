class('AnimatedSprite').extends(playdate.graphics.sprite)

local defaultOptions <const> = { frame = 1, fps = 12, runOnce = false }

function AnimatedSprite:init(imgTable, options)
  AnimatedSprite.super.init(self, imgTable[1])
  self:setImageTable(imgTable, options)
end

function AnimatedSprite:setImageTable(imgTable, options) 
  if self.timer then
    self.timer:remove()
  end

  local optionsToUse <const> = options or defaultOptions
  self.imgTable = imgTable
  self.frame = optionsToUse.frame or 1
  self.fps = optionsToUse.fps or 12
  self.loop = not optionsToUse.runOnce

  self:setImage(self.imgTable[self.frame])

  local thisSprite <const> = self
  local animationTimer <const> = playdate.timer.new(
    1000 / self.fps,
    function ()
      if thisSprite.frame >= #thisSprite.imgTable then
        thisSprite.frame = 1
      else
        thisSprite.frame = thisSprite.frame + 1
      end
      thisSprite:setImage(thisSprite.imgTable[thisSprite.frame])
    end
  )
  animationTimer.repeats = self.loop
  self.timer = animationTimer
end

