import "core/animated-sprite"

local idleImgTable <const> = playdate.graphics.imagetable.new("img/ant-idle")

class('Ant').extends(AnimatedSprite)

function Ant:init() 
  Ant.super.init(self, idleImgTable, 8)
end

