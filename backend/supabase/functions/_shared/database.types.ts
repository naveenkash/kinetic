export type Json =
  | string
  | number
  | boolean
  | null
  | { [key: string]: Json | undefined }
  | Json[];

export type Database = {
  api: {
    Tables: {
      [_ in never]: never;
    };
    Views: {
      my_account: {
        Row: {
          account_id: string | null;
          created_at: string | null;
          display_name: string | null;
          grade: number | null;
          status: string | null;
        };
        Relationships: [];
      };
      my_crew: {
        Row: {
          account_id: string | null;
          chat_topic: string | null;
          crew_id: string | null;
          display_name: string | null;
          face: string | null;
          harness: string | null;
          join_code: string | null;
          joined_at: string | null;
          leader_id: string | null;
          ready: boolean | null;
        };
        Relationships: [
          {
            foreignKeyName: "crew_members_account_id_fkey";
            columns: ["account_id"];
            isOneToOne: true;
            referencedRelation: "my_account";
            referencedColumns: ["account_id"];
          },
          {
            foreignKeyName: "crews_leader_id_fkey";
            columns: ["leader_id"];
            isOneToOne: false;
            referencedRelation: "my_account";
            referencedColumns: ["account_id"];
          },
        ];
      };
      my_invites: {
        Row: {
          crew_id: string | null;
          expires_at: string | null;
          from_id: string | null;
        };
        Insert: {
          crew_id?: string | null;
          expires_at?: string | null;
          from_id?: string | null;
        };
        Update: {
          crew_id?: string | null;
          expires_at?: string | null;
          from_id?: string | null;
        };
        Relationships: [
          {
            foreignKeyName: "crew_invites_crew_id_fkey";
            columns: ["crew_id"];
            isOneToOne: false;
            referencedRelation: "my_crew";
            referencedColumns: ["crew_id"];
          },
          {
            foreignKeyName: "crew_invites_from_id_fkey";
            columns: ["from_id"];
            isOneToOne: false;
            referencedRelation: "my_account";
            referencedColumns: ["account_id"];
          },
        ];
      };
    };
    Functions: {
      crew_accept: { Args: { p_crew: string }; Returns: Json };
      crew_create: { Args: never; Returns: Json };
      crew_decline: { Args: { p_crew: string }; Returns: undefined };
      crew_invite: { Args: { p_to: string }; Returns: undefined };
      crew_join_by_code: { Args: { p_code: string }; Returns: Json };
      crew_kick: { Args: { p_account: string }; Returns: undefined };
      crew_leave: { Args: never; Returns: undefined };
      crew_promote: { Args: { p_account: string }; Returns: undefined };
      crew_set_pick: {
        Args: { p_face?: string; p_harness: string; p_ready?: boolean };
        Returns: Json;
      };
      friends_resolve: {
        Args: { p_steam_ids: string[] };
        Returns: {
          account_id: string;
          display_name: string;
          state: string;
          steam_id: string;
        }[];
      };
      get_config: { Args: never; Returns: Json };
      presence_heartbeat: { Args: never; Returns: Json };
      queue_cancel: { Args: never; Returns: undefined };
      queue_join: {
        Args: { p_build: string; p_pings: Json; p_playlist: string };
        Returns: Json;
      };
      srv_build_ok: { Args: { p_build: string }; Returns: boolean };
      srv_current_match: { Args: { p_account: string }; Returns: Json };
      srv_ensure_account: {
        Args: {
          p_display_name?: string;
          p_provider: string;
          p_provider_user_id: string;
          p_user: string;
        };
        Returns: Json;
      };
      srv_find_account: {
        Args: { p_provider: string; p_provider_user_id: string };
        Returns: string;
      };
      srv_find_auth_user: { Args: { p_email: string }; Returns: string };
      srv_heartbeat: {
        Args: {
          p_allocation: string;
          p_connected: string[];
          p_match: string;
          p_state: string;
        };
        Returns: Json;
      };
      srv_manifest: {
        Args: { p_allocation: string; p_match: string };
        Returns: Json;
      };
      srv_player_event: {
        Args: {
          p_account: string;
          p_allocation: string;
          p_event: string;
          p_match: string;
        };
        Returns: undefined;
      };
      srv_rate_limit: {
        Args: { p_key: string; p_max: number; p_window_s: number };
        Returns: boolean;
      };
      srv_store_report: {
        Args: { p_allocation: string; p_match: string; p_payload: Json };
        Returns: Json;
      };
    };
    Enums: {
      [_ in never]: never;
    };
    CompositeTypes: {
      [_ in never]: never;
    };
  };
};

type DatabaseWithoutInternals = Omit<Database, "__InternalSupabase">;

type DefaultSchema = DatabaseWithoutInternals[Extract<keyof Database, "public">];

export type Tables<
  DefaultSchemaTableNameOrOptions extends
    | keyof (DefaultSchema["Tables"] & DefaultSchema["Views"])
    | { schema: keyof DatabaseWithoutInternals },
  TableName extends DefaultSchemaTableNameOrOptions extends {
    schema: keyof DatabaseWithoutInternals;
  } ? keyof (
      & DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Tables"]
      & DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Views"]
    )
    : never = never,
> = DefaultSchemaTableNameOrOptions extends {
  schema: keyof DatabaseWithoutInternals;
} ? (
    & DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Tables"]
    & DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Views"]
  )[TableName] extends {
    Row: infer R;
  } ? R
  : never
  : DefaultSchemaTableNameOrOptions extends keyof (
    & DefaultSchema["Tables"]
    & DefaultSchema["Views"]
  ) ? (
      & DefaultSchema["Tables"]
      & DefaultSchema["Views"]
    )[DefaultSchemaTableNameOrOptions] extends {
      Row: infer R;
    } ? R
    : never
  : never;

export type TablesInsert<
  DefaultSchemaTableNameOrOptions extends
    | keyof DefaultSchema["Tables"]
    | { schema: keyof DatabaseWithoutInternals },
  TableName extends DefaultSchemaTableNameOrOptions extends {
    schema: keyof DatabaseWithoutInternals;
  } ? keyof DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Tables"]
    : never = never,
> = DefaultSchemaTableNameOrOptions extends {
  schema: keyof DatabaseWithoutInternals;
} ? DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Tables"][TableName] extends {
    Insert: infer I;
  } ? I
  : never
  : DefaultSchemaTableNameOrOptions extends keyof DefaultSchema["Tables"]
    ? DefaultSchema["Tables"][DefaultSchemaTableNameOrOptions] extends {
      Insert: infer I;
    } ? I
    : never
  : never;

export type TablesUpdate<
  DefaultSchemaTableNameOrOptions extends
    | keyof DefaultSchema["Tables"]
    | { schema: keyof DatabaseWithoutInternals },
  TableName extends DefaultSchemaTableNameOrOptions extends {
    schema: keyof DatabaseWithoutInternals;
  } ? keyof DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Tables"]
    : never = never,
> = DefaultSchemaTableNameOrOptions extends {
  schema: keyof DatabaseWithoutInternals;
} ? DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Tables"][TableName] extends {
    Update: infer U;
  } ? U
  : never
  : DefaultSchemaTableNameOrOptions extends keyof DefaultSchema["Tables"]
    ? DefaultSchema["Tables"][DefaultSchemaTableNameOrOptions] extends {
      Update: infer U;
    } ? U
    : never
  : never;

export type Enums<
  DefaultSchemaEnumNameOrOptions extends
    | keyof DefaultSchema["Enums"]
    | { schema: keyof DatabaseWithoutInternals },
  EnumName extends DefaultSchemaEnumNameOrOptions extends {
    schema: keyof DatabaseWithoutInternals;
  } ? keyof DatabaseWithoutInternals[DefaultSchemaEnumNameOrOptions["schema"]]["Enums"]
    : never = never,
> = DefaultSchemaEnumNameOrOptions extends {
  schema: keyof DatabaseWithoutInternals;
} ? DatabaseWithoutInternals[DefaultSchemaEnumNameOrOptions["schema"]]["Enums"][EnumName]
  : DefaultSchemaEnumNameOrOptions extends keyof DefaultSchema["Enums"]
    ? DefaultSchema["Enums"][DefaultSchemaEnumNameOrOptions]
  : never;

export type CompositeTypes<
  PublicCompositeTypeNameOrOptions extends
    | keyof DefaultSchema["CompositeTypes"]
    | { schema: keyof DatabaseWithoutInternals },
  CompositeTypeName extends PublicCompositeTypeNameOrOptions extends {
    schema: keyof DatabaseWithoutInternals;
  } ? keyof DatabaseWithoutInternals[PublicCompositeTypeNameOrOptions["schema"]]["CompositeTypes"]
    : never = never,
> = PublicCompositeTypeNameOrOptions extends {
  schema: keyof DatabaseWithoutInternals;
} ? DatabaseWithoutInternals[PublicCompositeTypeNameOrOptions["schema"]]["CompositeTypes"][CompositeTypeName]
  : PublicCompositeTypeNameOrOptions extends keyof DefaultSchema["CompositeTypes"]
    ? DefaultSchema["CompositeTypes"][PublicCompositeTypeNameOrOptions]
  : never;

export const Constants = {
  api: {
    Enums: {},
  },
} as const;
