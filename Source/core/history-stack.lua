class('HistoryStack').extends()

function HistoryStack:init(size)
  self.stack = {}
  self.pos = 1
  self.size = size
end

function HistoryStack:push(item)
  self.stack[self.pos] = item
  self.pos = self.pos < self.size and self.pos + 1 or 1
end

function HistoryStack:pop()
  local pastPos = self.pos > 1 and self.pos - 1 or self.size
  if not self.stack[pastPos] then
    return nil
  end

  local item = self.stack[pastPos]
  self.stack[pastPos] = nil
  self.pos = pastPos
  return item
end

function HistoryStack:clear()
  for i=1,self.size do
    self.stack[i] = nil
  end
  self.pos = 1
end
