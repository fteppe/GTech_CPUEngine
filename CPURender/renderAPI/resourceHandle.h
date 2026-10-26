#pragma once
#include <functional>

template<typename TResource>
class ResourceHandle {
	size_t resourceId;
public:
	ResourceHandle() : resourceId(-1) {}
	bool isValid() const {return resourceId != -1;}
	bool operator==(const ResourceHandle & right) const {return resourceId == right.resourceId;}
private:
	friend struct std::hash<ResourceHandle>;
	friend class RenderAPI;
	ResourceHandle(size_t id) : resourceId(id) {}
};


template<typename TResource>
struct std::hash<ResourceHandle<TResource>> {
	std::size_t operator()(ResourceHandle<TResource> const& s) const noexcept {
		return std::hash<int>{}(s.resourceId);
	}
};