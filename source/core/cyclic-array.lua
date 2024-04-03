
class('CyclicArray').extends()

function CyclicArray:init(size, elementAllocator)
  self.nextIndex = 1
  self.size = size
  self.array = {}
  for i=1,size do
    self.array[i] = elementAllocator()
  end
end

function CyclicArray:nextElement()
  local element = self.array[self.nextIndex]
  self.nextIndex = self.nextIndex < self.size and self.nextIndex + 1 or 1
  return element
end
