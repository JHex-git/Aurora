# Percentage-Closer Filtering (PCF)

This document describes the PCF shadow filtering algorithm used for point and directional lights.

**Overview**
PCF softens hard shadow edges by sampling a small neighborhood around the nominal shadow lookup and averaging the binary shadow tests. This turns a single hard comparison into a filtered visibility value while preserving the original depth test logic.
- Kernel size $r$ is odd and configurable (1, 3, 5, 7, 9).
- The final visibility is the average of $r^2$ binary comparisons.

**Stage 1: Shared Early Exits**
- If a fragment is back-facing to the light ($\mathbf{n}\cdot\mathbf{l} \le 0$), visibility is 0.
- For point lights, if the fragment is beyond the light's far range, visibility is 1.

**Stage 2: Point Light PCF (Tangent-Plane Sampling)**
- Build a tangent frame around the light ray:
  - $\mathbf{d} = -\mathbf{l}$ (from light toward fragment)
  - $\mathbf{t} = \text{normalize}(\mathbf{up} \times \mathbf{d})$
  - $\mathbf{b} = \mathbf{d} \times \mathbf{t}$
- For each offset $(i, j)$ in the $r \times r$ grid (in texel units), form the sampling direction:
  $$
  \mathbf{s} = \text{normalize}(\mathbf{d} + i\,\mathbf{t} + j\,\mathbf{b})
  $$
- Sample the cube shadow map at direction $\mathbf{s}$ and recover radial distance:
  $$
  d_\text{occ} = \frac{\text{Linearize}(z_{01}, z_n, z_f)}{\max(|s_x|, |s_y|, |s_z|)}
  $$
- Compare against the fragment's radial distance $d_\text{cur}$ using a per-sample bias.

**Point Light Bias (Per-Sample)**
- Base bias (slope-like) uses the sample direction:
  $$
  b_\text{base} = \text{clamp}\left(\text{factor}\cdot(1 - \max(0, -\mathbf{n}\cdot\mathbf{s})),\, b_\text{min},\, b_\text{max}\right)
  $$
- Additional plane-compensation bias removes self-shadowing caused by sampling toward more head-on directions:
  $$
  t = -d_\text{cur}\,\frac{\mathbf{n}\cdot\mathbf{l}}{\mathbf{n}\cdot\mathbf{s}}, \quad
  b_\text{plane} = \max(0, d_\text{cur} - t)
  $$
- Final per-sample bias:
  $$
  b = b_\text{base} + b_\text{plane}
  $$

**Stage 3: Directional Light PCF**
- Select cascade based on view-space depth.
- Project the fragment into light space to get UV and light-space depth.
- For each offset $(i, j)$ in the $r \times r$ grid (in texel units):
  - If the UV is outside the shadow map, treat as lit.
  - Otherwise compare light-space depth against the sampled depth plus bias.

**Directional Light Bias**
- Use a slope-style bias based on $\mathbf{n}\cdot\mathbf{l}$:
  $$
  b = \text{clamp}\left(\text{factor}\cdot(1 - \mathbf{n}\cdot\mathbf{l}),\, b_\text{min},\, b_\text{max}\right)
  $$

**Visibility Aggregation**
- Average over the kernel:
  $$
  V = \frac{1}{r^2}\sum_{k=1}^{r^2} \mathbf{1}\left(d_\text{cur} < d_\text{occ}^{(k)} + b^{(k)}\right)
  $$
- Visibility $V$ directly modulates diffuse and specular lighting.
