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
      catalog: {
        Row: {
          grade_gated: number | null;
          item_id: string | null;
          metadata: Json | null;
          slot: string | null;
          source_rule: string | null;
        };
        Insert: {
          grade_gated?: number | null;
          item_id?: string | null;
          metadata?: Json | null;
          slot?: string | null;
          source_rule?: string | null;
        };
        Update: {
          grade_gated?: number | null;
          item_id?: string | null;
          metadata?: Json | null;
          slot?: string | null;
          source_rule?: string | null;
        };
        Relationships: [];
      };
      claim_standing: {
        Row: {
          active_players: number | null;
          claim_id: string | null;
          faction: string | null;
          refreshed_at: string | null;
          standing: number | null;
        };
        Insert: {
          active_players?: number | null;
          claim_id?: string | null;
          faction?: string | null;
          refreshed_at?: string | null;
          standing?: never;
        };
        Update: {
          active_players?: number | null;
          claim_id?: string | null;
          faction?: string | null;
          refreshed_at?: string | null;
          standing?: never;
        };
        Relationships: [
          {
            foreignKeyName: "claim_standing_claim_id_fkey";
            columns: ["claim_id"];
            isOneToOne: false;
            referencedRelation: "claims";
            referencedColumns: ["claim_id"];
          },
        ];
      };
      claims: {
        Row: {
          arena: string | null;
          claim_id: string | null;
          ends_at: string | null;
          name: string | null;
          outcome: Json | null;
          starts_at: string | null;
          state: string | null;
        };
        Insert: {
          arena?: string | null;
          claim_id?: string | null;
          ends_at?: string | null;
          name?: string | null;
          outcome?: never;
          starts_at?: string | null;
          state?: string | null;
        };
        Update: {
          arena?: string | null;
          claim_id?: string | null;
          ends_at?: string | null;
          name?: string | null;
          outcome?: never;
          starts_at?: string | null;
          state?: string | null;
        };
        Relationships: [];
      };
      current_ruleset: {
        Row: {
          grade_thresholds_t: Json | null;
          rope_mark_tiers: Json | null;
          tonnage: Json | null;
          version: number | null;
          weight_start_t: number | null;
        };
        Insert: {
          grade_thresholds_t?: never;
          rope_mark_tiers?: never;
          tonnage?: never;
          version?: number | null;
          weight_start_t?: never;
        };
        Update: {
          grade_thresholds_t?: never;
          rope_mark_tiers?: never;
          tonnage?: never;
          version?: number | null;
          weight_start_t?: never;
        };
        Relationships: [];
      };
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
      my_account_jobs: {
        Row: {
          completed_at: string | null;
          job_id: string | null;
          kind: string | null;
          requested_at: string | null;
          result_path: string | null;
          run_after: string | null;
          status: string | null;
        };
        Insert: {
          completed_at?: string | null;
          job_id?: string | null;
          kind?: string | null;
          requested_at?: string | null;
          result_path?: string | null;
          run_after?: string | null;
          status?: string | null;
        };
        Update: {
          completed_at?: string | null;
          job_id?: string | null;
          kind?: string | null;
          requested_at?: string | null;
          result_path?: string | null;
          run_after?: string | null;
          status?: string | null;
        };
        Relationships: [];
      };
      my_allegiance: {
        Row: {
          claim_id: string | null;
          declared_at: string | null;
          faction: string | null;
        };
        Insert: {
          claim_id?: string | null;
          declared_at?: string | null;
          faction?: string | null;
        };
        Update: {
          claim_id?: string | null;
          declared_at?: string | null;
          faction?: string | null;
        };
        Relationships: [
          {
            foreignKeyName: "allegiances_claim_id_fkey";
            columns: ["claim_id"];
            isOneToOne: false;
            referencedRelation: "claims";
            referencedColumns: ["claim_id"];
          },
        ];
      };
      my_career_stats: {
        Row: {
          airborne_seconds: number | null;
          assists: number | null;
          floor_seconds: number | null;
          harness: string | null;
          hoisted_seconds: number | null;
          mass_distance_kg_m: number | null;
          matches: number | null;
          rated_tons_moved: number | null;
          ringouts: number | null;
          scraps: number | null;
          spotter_credits: number | null;
          wins: number | null;
        };
        Insert: {
          airborne_seconds?: number | null;
          assists?: number | null;
          floor_seconds?: number | null;
          harness?: string | null;
          hoisted_seconds?: number | null;
          mass_distance_kg_m?: number | null;
          matches?: number | null;
          rated_tons_moved?: number | null;
          ringouts?: number | null;
          scraps?: number | null;
          spotter_credits?: number | null;
          wins?: number | null;
        };
        Update: {
          airborne_seconds?: number | null;
          assists?: number | null;
          floor_seconds?: number | null;
          harness?: string | null;
          hoisted_seconds?: number | null;
          mass_distance_kg_m?: number | null;
          matches?: number | null;
          rated_tons_moved?: number | null;
          ringouts?: number | null;
          scraps?: number | null;
          spotter_credits?: number | null;
          wins?: number | null;
        };
        Relationships: [];
      };
      my_contracts: {
        Row: {
          claim_id: string | null;
          completed_at: string | null;
          contract_id: string | null;
          counter_key: string | null;
          faction: string | null;
          progress: number | null;
          reward: Json | null;
          target: number | null;
          title: string | null;
          week: number | null;
        };
        Relationships: [
          {
            foreignKeyName: "contracts_claim_id_fkey";
            columns: ["claim_id"];
            isOneToOne: false;
            referencedRelation: "claims";
            referencedColumns: ["claim_id"];
          },
        ];
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
      my_entitlements: {
        Row: {
          entitlement: string | null;
          granted_at: string | null;
        };
        Insert: {
          entitlement?: string | null;
          granted_at?: string | null;
        };
        Update: {
          entitlement?: string | null;
          granted_at?: string | null;
        };
        Relationships: [];
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
      my_items: {
        Row: {
          acquired_at: string | null;
          item_id: string | null;
          slot: string | null;
          source: string | null;
        };
        Relationships: [];
      };
      my_ledger: {
        Row: {
          account_id: string | null;
          cleared_at: string | null;
          cutter_issued_at: string | null;
          cutter_serial: string | null;
          grade: number | null;
          tonnage_total: number | null;
          updated_at: string | null;
          weight_balance: number | null;
        };
        Insert: {
          account_id?: string | null;
          cleared_at?: string | null;
          cutter_issued_at?: string | null;
          cutter_serial?: string | null;
          grade?: number | null;
          tonnage_total?: number | null;
          updated_at?: string | null;
          weight_balance?: number | null;
        };
        Update: {
          account_id?: string | null;
          cleared_at?: string | null;
          cutter_issued_at?: string | null;
          cutter_serial?: string | null;
          grade?: number | null;
          tonnage_total?: number | null;
          updated_at?: string | null;
          weight_balance?: number | null;
        };
        Relationships: [
          {
            foreignKeyName: "ledgers_account_id_fkey";
            columns: ["account_id"];
            isOneToOne: true;
            referencedRelation: "my_account";
            referencedColumns: ["account_id"];
          },
        ];
      };
      my_ledger_entries: {
        Row: {
          entry_id: string | null;
          match_id: string | null;
          unlocked_at: string | null;
        };
        Insert: {
          entry_id?: string | null;
          match_id?: string | null;
          unlocked_at?: string | null;
        };
        Update: {
          entry_id?: string | null;
          match_id?: string | null;
          unlocked_at?: string | null;
        };
        Relationships: [];
      };
      my_loadouts: {
        Row: {
          cosmetics: Json | null;
          cutter: string | null;
          dog: string | null;
          harness: string | null;
          line: string | null;
          updated_at: string | null;
        };
        Insert: {
          cosmetics?: Json | null;
          cutter?: string | null;
          dog?: string | null;
          harness?: string | null;
          line?: string | null;
          updated_at?: string | null;
        };
        Update: {
          cosmetics?: Json | null;
          cutter?: string | null;
          dog?: string | null;
          harness?: string | null;
          line?: string | null;
          updated_at?: string | null;
        };
        Relationships: [
          {
            foreignKeyName: "loadouts_cutter_fkey";
            columns: ["cutter"];
            isOneToOne: false;
            referencedRelation: "catalog";
            referencedColumns: ["item_id"];
          },
          {
            foreignKeyName: "loadouts_cutter_fkey";
            columns: ["cutter"];
            isOneToOne: false;
            referencedRelation: "my_items";
            referencedColumns: ["item_id"];
          },
          {
            foreignKeyName: "loadouts_dog_fkey";
            columns: ["dog"];
            isOneToOne: false;
            referencedRelation: "catalog";
            referencedColumns: ["item_id"];
          },
          {
            foreignKeyName: "loadouts_dog_fkey";
            columns: ["dog"];
            isOneToOne: false;
            referencedRelation: "my_items";
            referencedColumns: ["item_id"];
          },
          {
            foreignKeyName: "loadouts_line_fkey";
            columns: ["line"];
            isOneToOne: false;
            referencedRelation: "catalog";
            referencedColumns: ["item_id"];
          },
          {
            foreignKeyName: "loadouts_line_fkey";
            columns: ["line"];
            isOneToOne: false;
            referencedRelation: "my_items";
            referencedColumns: ["item_id"];
          },
        ];
      };
      my_manifest: {
        Row: {
          claim_id: string | null;
          earned: boolean | null;
          reward: Json | null;
          tier: number | null;
          tonnage_earned: number | null;
          tonnage_t: number | null;
          track: string | null;
        };
        Relationships: [
          {
            foreignKeyName: "manifest_crates_claim_id_fkey";
            columns: ["claim_id"];
            isOneToOne: false;
            referencedRelation: "claims";
            referencedColumns: ["claim_id"];
          },
        ];
      };
      my_match_summaries: {
        Row: {
          breakdown: Json | null;
          contracts: Json | null;
          created_at: string | null;
          credit: number | null;
          grade_after: number | null;
          grade_before: number | null;
          manifest: Json | null;
          match_id: string | null;
          rating: Json | null;
          rope_marks: Json | null;
          ruleset: number | null;
          standing: Json | null;
          tonnage: number | null;
          unlocks: Json | null;
          weight_after: number | null;
          weight_before: number | null;
          win: boolean | null;
        };
        Insert: {
          breakdown?: Json | null;
          contracts?: Json | null;
          created_at?: string | null;
          credit?: number | null;
          grade_after?: number | null;
          grade_before?: number | null;
          manifest?: Json | null;
          match_id?: string | null;
          rating?: Json | null;
          rope_marks?: Json | null;
          ruleset?: number | null;
          standing?: Json | null;
          tonnage?: number | null;
          unlocks?: Json | null;
          weight_after?: number | null;
          weight_before?: number | null;
          win?: boolean | null;
        };
        Update: {
          breakdown?: Json | null;
          contracts?: Json | null;
          created_at?: string | null;
          credit?: number | null;
          grade_after?: number | null;
          grade_before?: number | null;
          manifest?: Json | null;
          match_id?: string | null;
          rating?: Json | null;
          rope_marks?: Json | null;
          ruleset?: number | null;
          standing?: Json | null;
          tonnage?: number | null;
          unlocks?: Json | null;
          weight_after?: number | null;
          weight_before?: number | null;
          win?: boolean | null;
        };
        Relationships: [];
      };
      my_purchases: {
        Row: {
          amount: number | null;
          created_at: string | null;
          currency_code: string | null;
          finalized_at: string | null;
          order_id: string | null;
          sku: string | null;
          status: string | null;
        };
        Insert: {
          amount?: number | null;
          created_at?: string | null;
          currency_code?: string | null;
          finalized_at?: string | null;
          order_id?: never;
          sku?: string | null;
          status?: string | null;
        };
        Update: {
          amount?: number | null;
          created_at?: string | null;
          currency_code?: string | null;
          finalized_at?: string | null;
          order_id?: never;
          sku?: string | null;
          status?: string | null;
        };
        Relationships: [
          {
            foreignKeyName: "purchases_sku_fkey";
            columns: ["sku"];
            isOneToOne: false;
            referencedRelation: "store";
            referencedColumns: ["sku"];
          },
        ];
      };
      my_ratings: {
        Row: {
          band: string | null;
          games: number | null;
          mode: string | null;
          updated_at: string | null;
        };
        Insert: {
          band?: string | null;
          games?: number | null;
          mode?: string | null;
          updated_at?: string | null;
        };
        Update: {
          band?: string | null;
          games?: number | null;
          mode?: string | null;
          updated_at?: string | null;
        };
        Relationships: [];
      };
      my_rope_marks: {
        Row: {
          count: number | null;
          rig: string | null;
          tier: string | null;
          updated_at: string | null;
        };
        Insert: {
          count?: number | null;
          rig?: string | null;
          tier?: string | null;
          updated_at?: string | null;
        };
        Update: {
          count?: number | null;
          rig?: string | null;
          tier?: string | null;
          updated_at?: string | null;
        };
        Relationships: [];
      };
      my_standing: {
        Row: {
          claim_id: string | null;
          points: number | null;
          reason: string | null;
          week: number | null;
        };
        Relationships: [
          {
            foreignKeyName: "claim_standing_events_claim_id_fkey";
            columns: ["claim_id"];
            isOneToOne: false;
            referencedRelation: "claims";
            referencedColumns: ["claim_id"];
          },
        ];
      };
      my_wallet_txns: {
        Row: {
          created_at: string | null;
          currency: string | null;
          delta: number | null;
          reason: string | null;
          ref: string | null;
          txn_id: string | null;
        };
        Insert: {
          created_at?: string | null;
          currency?: string | null;
          delta?: number | null;
          reason?: string | null;
          ref?: string | null;
          txn_id?: string | null;
        };
        Update: {
          created_at?: string | null;
          currency?: string | null;
          delta?: number | null;
          reason?: string | null;
          ref?: string | null;
          txn_id?: string | null;
        };
        Relationships: [];
      };
      my_wallets: {
        Row: {
          balance: number | null;
          currency: string | null;
        };
        Insert: {
          balance?: number | null;
          currency?: string | null;
        };
        Update: {
          balance?: number | null;
          currency?: string | null;
        };
        Relationships: [];
      };
      shop: {
        Row: {
          claim_id: string | null;
          currency: string | null;
          ends_at: string | null;
          item_id: string | null;
          price: number | null;
          slot: string | null;
          starts_at: string | null;
        };
        Relationships: [
          {
            foreignKeyName: "shop_rotation_item_id_fkey";
            columns: ["item_id"];
            isOneToOne: false;
            referencedRelation: "catalog";
            referencedColumns: ["item_id"];
          },
          {
            foreignKeyName: "shop_rotation_item_id_fkey";
            columns: ["item_id"];
            isOneToOne: false;
            referencedRelation: "my_items";
            referencedColumns: ["item_id"];
          },
        ];
      };
      store: {
        Row: {
          claim_id: string | null;
          credit: number | null;
          description: string | null;
          kind: string | null;
          prices: Json | null;
          sku: string | null;
        };
        Insert: {
          claim_id?: string | null;
          credit?: number | null;
          description?: string | null;
          kind?: string | null;
          prices?: Json | null;
          sku?: string | null;
        };
        Update: {
          claim_id?: string | null;
          credit?: number | null;
          description?: string | null;
          kind?: string | null;
          prices?: Json | null;
          sku?: string | null;
        };
        Relationships: [
          {
            foreignKeyName: "skus_claim_id_fkey";
            columns: ["claim_id"];
            isOneToOne: false;
            referencedRelation: "claims";
            referencedColumns: ["claim_id"];
          },
        ];
      };
    };
    Functions: {
      cancel_deletion: { Args: never; Returns: Json };
      change_display_name: { Args: { p_name: string }; Returns: Json };
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
      declare_allegiance: { Args: { p_faction: string }; Returns: Json };
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
      report_player: {
        Args: {
          p_match: string;
          p_reason: string;
          p_reported: string;
          p_text?: string;
        };
        Returns: Json;
      };
      request_deletion: { Args: never; Returns: Json };
      request_export: { Args: never; Returns: Json };
      set_loadout: {
        Args: {
          p_cosmetics?: Json;
          p_cutter: string;
          p_dog: string;
          p_harness: string;
          p_line: string;
        };
        Returns: Json;
      };
      shop_purchase: {
        Args: { p_expected_price: number; p_idem: string; p_item: string };
        Returns: Json;
      };
      srv_build_ok: { Args: { p_build: string }; Returns: boolean };
      srv_claim_resolve: { Args: { p_claim: string }; Returns: Json };
      srv_claim_review: {
        Args: {
          p_claim: string;
          p_holder?: string;
          p_staff: string;
          p_variant?: string;
        };
        Returns: Json;
      };
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
      srv_link_identity: {
        Args: {
          p_account: string;
          p_provider: string;
          p_provider_user_id: string;
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
      srv_player_loadout: {
        Args: { p_account: string; p_allocation: string; p_match: string };
        Returns: Json;
      };
      srv_purchase_checked: { Args: { p_order: number }; Returns: undefined };
      srv_purchase_create: {
        Args: { p_account: string; p_currency: string; p_sku: string };
        Returns: Json;
      };
      srv_purchase_failed: {
        Args: { p_order: number; p_reason: string; p_status: string };
        Returns: undefined;
      };
      srv_purchase_finalize: {
        Args: { p_order: number; p_trans: string };
        Returns: Json;
      };
      srv_purchase_flag: {
        Args: { p_order: number; p_reason: string };
        Returns: undefined;
      };
      srv_purchase_initiated: {
        Args: { p_order: number; p_trans: string };
        Returns: undefined;
      };
      srv_purchase_of: {
        Args: { p_account: string; p_order: number };
        Returns: Json;
      };
      srv_purchase_reverse: {
        Args: { p_order: number; p_status: string };
        Returns: Json;
      };
      srv_rate_limit: {
        Args: { p_key: string; p_max: number; p_window_s: number };
        Returns: boolean;
      };
      srv_report_handle: {
        Args: { p_report: string; p_staff: string; p_status: string };
        Returns: Json;
      };
      srv_requeue_report: { Args: { p_match: string }; Returns: Json };
      srv_reset_display_name: { Args: { p_account: string }; Returns: Json };
      srv_steam_id: { Args: { p_account: string }; Returns: string };
      srv_store_report: {
        Args: { p_allocation: string; p_match: string; p_payload: Json };
        Returns: Json;
      };
      srv_unlink_identity: {
        Args: { p_account: string; p_provider: string };
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
