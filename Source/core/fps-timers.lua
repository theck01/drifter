
local timer <const> = playdate.timer
local timerMap <const> = {}
local idToFPSMap <const> = {}
local callbackIdCounter = 1

class('FPSTimer').extends()
local klass <const> = FPSTimer

function klass.add(callback, fps, runOnce)
  if not timerMap[fps] then
    timerMap[fps] = {
      t = timer.new(1000 / fps, klass._processTimerFired, fps),
      paused = false,
      callbacks = {}
    }
    timerMap[fps].t.repeats = true
  end

  local timerRecord <const> = timerMap[fps]

  local newId = callbackIdCounter
  callbackIdCounter += 1

  table.insert(timerRecord.callbacks, {
    id = newId,
    cb = callback,
    runOnce = not not runOnce
  })
  idToFPSMap[newId] = fps

  if timerRecord.paused then
    timerRecord.t:start()
    timerRecord.paused = false
  end
end

function klass.remove(callbackId) 
  local fps = idToFPSMap[callbackId]
  if not fps then
    error('Cannot reset callback ' .. callbackId .. ' , no FPS timer found')
  end
  idToFPSMap[callbackId] = nil

  local timerRecord = timerMap[fps]
  if not timerRecord then 
    error('Cannot reset callback for non-existant FPS timer, ' .. fps .. ' fps')
  end

  for i=1,#timerRecord.callbacks do
    if timerRecord.callbacks[i].id == callbackId then
      table.remove(timerRecord.callbacks, i)
      if #timerRecord.callbacks == 0 then
        timerRecord.t:pause()
        timerRecord.paused = true
      end
      return
    end
  end
  error('Cannot reset callback for non-existant id, ' .. callbackId)
end

function klass._processTimerFired(fps)
  local timerRecord <const> = timerMap[fps]
  if not timerMap[fps] then 
    error('Cannot process non-existant FPS timer for ' .. fps .. ' fps')
  end


  for i=1, #timerRecord.callbacks do
    timerRecord.callbacks[i].cb()
    if timerRecord.callbacks[i].runOnce then
      klass.remove(callbacks[i].id)
    end
  end
end
