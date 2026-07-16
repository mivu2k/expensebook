#include "UndoService.h"

namespace eb {

void UndoService::pushDeleted(Deleted d)
{
    m_stack.push(std::move(d));
    emit availabilityChanged(true);
}

UndoService::Deleted UndoService::pop()
{
    Deleted d = m_stack.pop();
    emit availabilityChanged(!m_stack.isEmpty());
    return d;
}

} // namespace eb
