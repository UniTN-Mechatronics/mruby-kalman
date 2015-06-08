#*************************************************************************#
#                                                                         #
# kalman.rb - Kalman filtering for mruby                                  #
# Copyright (C) 2015 Paolo Bosetti and Mirko Brentari,                    #
# paolo[dot]bosetti[at]unitn.it                                           #
# Department of Industrial Engineering, University of Trento              #
#                                                                         #
# This library is free software.  You can redistribute it and/or          #
# modify it under the terms of the GNU GENERAL PUBLIC LICENSE 2.0.        #
#                                                                         #
# This library is distributed in the hope that it will be useful,         #
# but WITHOUT ANY WARRANTY; without even the implied warranty of          #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           #
# Artistic License 2.0 for more details.                                  #
#                                                                         #
# See the file LICENSE                                                    #
#                                                                         #
#*************************************************************************#

class Kalman
  def inspect
    s = self.to_s[0..-2]
    s << " @theta=#{self.theta} "
    s << "@thetad=#{self.thetad} "
    s << "@Q_theta=#{self.Q_theta} "
    s << "@Q_thetad=#{self.Q_thetad} "
    s << "@R=#{self.R} "
    s << "@P=#{self.P}"
    s << ">"
    return s
  end
end
