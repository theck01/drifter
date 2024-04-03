import "CoreLibs/object"
import "CoreLibs/graphics"

local counter = 0

function playdate.update()
  print(counter)
  counter += 1
end
