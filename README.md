# Sistema de automatizaciòn Pitot
En el presente repositorio se adjunta el código para manejar los motores de un mecanismo de elevación de una impresora de papel HP Deskjet Ink Advantage 1515, a fin de automatizar el proceso de medición en un túnel de viento con un tubo Pitot; además, se presenta una implementación de filtro de Kalman para estimar la velocidad y la presión del flujo.

Las ecuaciones diferenciales para el modelo en variables de estado, se derivan de las ecuaciones de Navier Stokes:

$$/rho/frac{/partial v}{/partial t+(v /dot /nabla v)} = -/nabla P+/mu /nabla^2 v + /rho g$$
