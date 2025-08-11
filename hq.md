portal science

xyw dimensions, portals can only link between other w levels
this still allows weird geometry A > B > C > A but simplifies
pathfinding and field of view calculations as we simply
change the w component instead of having to manage offsets
a hunch tells me that this also simplifies certain anomalies
as you can still determine the distance between objects
and with symetrical shadowcasting, if one subject sees the portal
we can garuntee the other side will see it. So if two objects
can see each other, we can simply calculate distance between them.

if portals have a minimum render distance that is less than the minimum
distance between portals, we can reduce the complexity that comes
with having "recursive" portals

pathfinding too, if an AI can see a point of interest through the portal
we simply move towards the portal, there is no need

if each w level is limited to a size of 100x100 tiles, the camera
can snap to the sides reducing the jankyness of a scrolling cam
along with a deadzone. It also makes each "zone" more cohesive to generate
in an episodic nature.
