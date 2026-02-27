# todo

**Introduce Renderer Abstraction**: Replace direct VulkanContext dependencies with IRenderer interface defining buffer creation, command recording, and resource queries. 
Implement VulkanRenderer and MockRenderer, enabling unit tests without GPU and potential DirectX/Metal backends. This aligns with dependency inversion principle.
