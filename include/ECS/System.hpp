#pragma once

template <typename... Components> StorageView<Components...> System::View() { return m_Storage->View<Components...>(); }
void                                                         System::Bind(Storage* storage) { m_Storage = storage; }
