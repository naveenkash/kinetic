-- Realtime Authorization for private channels (backend-architecture section 9.1).
-- user:<account_id>       receive only: invites, match_found, match_aborted, queue resets.
-- crew:<crew_id>          receive only: crew_state and queue_state, sent by the server.
-- crew-chat:<crew_id>:<chat_key>  members send and receive: crew chat and presence. The
--                         chat key rotates whenever a member leaves (private.leave_crew).
--
-- Realtime checks write access once per channel join, without the event name, so a topic
-- is either client-writable or not. Server events therefore never share a topic with
-- anything clients may send, and are sent with realtime.send from database functions and
-- the worker. Access is also cached per join, so removal is enforced by rotation: the join
-- code and chat key change whenever a member leaves, and the remaining members follow the
-- new chat_topic in crew_state. Clients also drop crew channels on crew_left and crew_kicked.

create policy kinetic_receive on realtime.messages
  for select to authenticated
  using (
    (select realtime.topic()) = 'user:' || (select auth.uid())::text
    or (select realtime.topic()) = 'crew:' || (select private.my_crew_id())::text
    or (select realtime.topic()) = (select private.my_crew_chat_topic())
  );

create policy kinetic_send_crew_chat on realtime.messages
  for insert to authenticated
  with check (
    (select realtime.topic()) = (select private.my_crew_chat_topic())
  );
