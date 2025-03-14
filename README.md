# Sistema de automatización Pitot
En el presente repositorio se adjunta el código para manejar los motores de un mecanismo de elevación de una impresora de papel HP Deskjet Ink Advantage 1515, a fin de automatizar el proceso de medición en un túnel de viento con un tubo Pitot; además, se presenta una implementación de filtro de Kalman para estimar la velocidad y la presión del flujo.

Las ecuaciones diferenciales para el modelo en variables de estado que describen el flujo dentro del túnel de viento, se derivan de las ecuaciones de Navier Stokes:

$$\rho \left( \frac{\partial v}{\partial t} + v \cdot \nabla v \right) = -\nabla P + \nabla \cdot T + f$$

Estas ecuaciones describen el comportamiento de fluidos en tres dimensiones; sin embargo, para el túnel de viento realizamos las siguientes suposiciones que facilitan demasiado el cálculo del modelo: 

+ Flujo laminar incompresible a lo largo del tubo.
+ Flujo solo en la dirección x, es decir $v_y = v_z = 0, \forall t>0$.

Con lo cual nos queda la siguiente ecuación diferencial:

$$\rho \left( \frac{\partial v_x}{\partial t} + v_x \frac{\partial v_x}{\partial x} \right) = -\frac{\partial P_x}{\partial x} + \mu \left( \frac{\partial^2 v_x}{\partial x^2}+\frac{\partial^2 v_x}{\partial y^2} \right) + \rho g_x$$
