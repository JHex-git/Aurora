            // For in-place write with branch, write-after-read cases exist, ？？？Read After write caused by alias
            // e.g. pass1 -> RT1, pass2 -> RT1 alias, pass3 reads RT1. pass3 must run before pass2
            if (resource->IsAlias())
            {
                auto producer_id = resource_producer_map[resource_handle.id];
                for (auto consumed_resource_handle : m_passes[producer_id]->m_consumed_resources)
                {
                    RenderGraphResource* consumed_resource = m_resources[consumed_resource_handle.id].get();
                    if (consumed_resource->GetAliasRoot() == resource->GetAliasRoot())
                    {
                        for (auto consumer_pass : consumed_resource->m_consumers)
                        {
                            pass_successors[consumer_pass->]

                        }
                    }
                }
            }

            OwnsResolved