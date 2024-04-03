class('AnimatedSprite').extends(playdate.graphics.sprite)

function AnimatedSprite:init(imgTable, fps, runOnce) 
  AnimatedSprite.super.init(self, imgTable[1])

  self.imgTable = imgTable
  self.frame = 1
  self.fps = fps or 12
  self.loop = not runOnce

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

