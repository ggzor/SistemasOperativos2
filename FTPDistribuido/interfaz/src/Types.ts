type ComputerInfo = {
  status: "online" | "replica" | "offline"
  replica: string
  syncState:
    | { state: "syncing"; total: number; progress: number }
    | { state: "synced" }
  fileCount: number
  totalFileSize: number
  sentFiles: number
  receivedFiles: number
}
