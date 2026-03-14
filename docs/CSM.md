# Cascaded Shadow Maps

This document gives a method overview of the CSM pipeline in Aurora, focusing on cascade splits, cascade selection during shading, and geometry culling for shadow rendering.

**Cascade Split Strategy**
- The camera frustum is divided into multiple depth ranges (cascades) between the camera near and far planes.
- Split positions are computed by blending linear and logarithmic distributions, concentrating more resolution closer to the camera while still covering distant geometry.
- Aurora uses $\lambda = 0.95$ for the split blend.

Split distances (for cascade index $i = 1..N$):
$$
\begin{aligned}
p_i &= \frac{i}{N} \\
d_{log} &= n \left(\frac{f}{n}\right)^{p_i} \\
d_{lin} &= n + (f - n) p_i \\
d_i &= \lambda d_{log} + (1 - \lambda) d_{lin}
\end{aligned}
$$
Here $n$ and $f$ are the camera near and far planes, and $\lambda$ blends log and linear splits. The intent is to allocate more resolution near the camera without ignoring far geometry.

**Per-Cascade Frustum Fitting**
- For each cascade, the eight view-space frustum corners of that slice are built from the split depths and camera FOV/aspect.
- Those corners are transformed into world space.
- A light-space view matrix is built to look at the cascade slice center.
- An orthographic projection is fitted to the light-space bounds of the slice, with padding to reduce shimmering and ensure coverage.
- The light-space XY projection is snapped to the shadow-map texel grid to stabilize shadows during camera movement.

Frustum slice dimensions at depth $d$:
$$
\begin{aligned}
h(d) &= 2 d \tan\left(\frac{fov}{2}\right) \\
w(d) &= h(d) \cdot aspect
\end{aligned}
$$

View-space frustum corners for a slice $[d_{near}, d_{far}]$ (camera looks down $-Z$):
$$
\begin{aligned}
z &= -d_{near} \text{ or } -d_{far} \\
x &= \pm \frac{w(d)}{2}, \quad y = \pm \frac{h(d)}{2}
\end{aligned}
$$

**Texel Snapping (Stabilization)**

When the camera moves, the light-space projection can shift by sub-texel amounts, causing visible shimmering. To prevent this, the light-space projection is aligned to the shadow-map texel grid.

Let the light-space XY bounds be $[x_{min}, x_{max}]$, $[y_{min}, y_{max}]$ and the shadow map size be $S \times S$ :
$$
\begin{aligned}
r_x &= x_{max} - x_{min} \\
r_y &= y_{max} - y_{min} \\
t_x &= \frac{r_x}{S}, \quad t_y = \frac{r_y}{S} \\
c_x &= \frac{x_{min} + x_{max}}{2}, \quad c_y = \frac{y_{min} + y_{max}}{2} \\
\hat{c}_x &= \mathrm{round}\left(\frac{c_x}{t_x}\right) t_x, \quad \hat{c}_y = \mathrm{round}\left(\frac{c_y}{t_y}\right) t_y
\end{aligned}
$$
Here $(c_x, c_y)$ is the light-space center of the cascade bounds, and $(\hat{c}_x, \hat{c}_y)$ is the snapped center.
Then rebuild the bounds to keep the same extents:
$$
\begin{aligned}
x_{min} &= \hat{c}_x - \frac{r_x}{2}, \quad x_{max} = \hat{c}_x + \frac{r_x}{2} \\
y_{min} &= \hat{c}_y - \frac{r_y}{2}, \quad y_{max} = \hat{c}_y + \frac{r_y}{2}
\end{aligned}
$$

**Shadow Rendering**
- Each cascade is rendered into its corresponding layer of the depth array.
- Shadow depth is generated using the cascade's light-space view and projection.

**Geometry Culling for CSM**
- A world-space AABB is derived for each cascade by transforming the light-space bounds back to world space.
- Each mesh's world-space AABB is tested against the cascade AABB.
- Meshes that do not intersect the cascade's AABB are skipped for that cascade's shadow rendering.

**Cascade Selection During Shading**
- The fragment's view-space depth is computed in the forward shader.
- The cascade whose split depth range contains the fragment depth is selected.
- The corresponding shadow map layer is sampled for the visibility test.

Cascade selection by view-space depth:
$$
\begin{aligned}
z_{view} &= - (V \cdot \text{vec4}(P_{world}, 1)).z \\
\text{select smallest } i \text{ such that } z_{view} &\le d_i
\end{aligned}
$$
