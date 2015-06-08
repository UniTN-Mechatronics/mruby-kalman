# mruby-kalman
Kalman filtering for mruby.

##Usage

```ruby
kf = Kalman.new(theta, thetad) # both arguments are optional)
p kf.P #Shows the P matrix
kf[0,0] = 10.0 # Sets first element of P
               # kf.theta, kf.thetad, kf.Q_tehta, kf.Q_thetad, kf.R also settable
kf.update(dt, theta, thetad) # => filtered theta
```
