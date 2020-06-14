import * as React from "react"
import { render } from "react-dom"
import styled, { css } from "styled-components"

import { ComputerCard } from "./components/ComputerCard"
import { useState } from "react"
import { useEffect } from "react"
import { LoadingIndicator } from "./components/LoadingIndicator"

window.receiver = null
window.receive = function (message) {
  if (receiver) receiver(message)
}

const App = ({}) => {
  const [state, setState] = useState({
    phase: "running",
    computers: [
      {
        name: "Computadora 1",
        fileSystem: {
          type: "directory",
          name: "files/folder1",
          size: 0,
          files: [
            { type: "file", name: "archivo1.txt", size: 0 },
            { type: "file", name: "archivo2.txt", size: 0 },
            { type: "file", name: "archivo3.txt", size: 0 },
            {
              type: "directory",
              name: "folder1.1",
              size: 0,
              files: [
                { type: "file", name: "archivo4.txt", size: 0 },
                { type: "file", name: "archivo5.txt", size: 0 },
                { type: "file", name: "archivo6.txt", size: 0 },
              ],
            },
          ],
        },
        info: {
          isLocal: true,
          status: "online",
          replica: "Computadora 2",
          syncState: { state: "synced" },
          fileCount: 6,
          totalFileSize: 0,
          sentFiles: null,
          receivedFiles: null,
        },
      },
      {
        name: "Computadora 2",
        fileSystem: null,
        info: {
          isLocal: false,
          status: "replica",
          replica: "Computadora 1",
          syncState: { state: "synced" },
          fileCount: null,
          totalFileSize: null,
          sentFiles: null,
          receivedFiles: null,
        },
      },
    ],
  })

  useEffect(() => {
    window.receiver = setState
    return () => (window.receiver = null)
  })

  const handleEvent = (event) => {
    console.log(JSON.stringify({ type: "message", message: event }))
  }

  return (
    <AppLayout {...state}>
      {state.phase !== "running" && (
        <CentralMessage>
          {state.phase === "initial"
            ? "Conectando con el servidor central"
            : state.phase === "named"
            ? "Esperando otros clientes"
            : ""}
        </CentralMessage>
      )}
      {state.phase === "running" &&
        state.computers.map((c) => (
          <ComputerCard key={c.name} {...c} onEvent={handleEvent} />
        ))}
    </AppLayout>
  )
}

const cssForPhase = {
  initial: css`
    align-items: center;
    justify-content: center;
    height: 100vh;
  `,
  running: css`
    margin: 4px;
    flex-wrap: wrap;

    & > * {
      margin: 12px;
    }
  `,
}

const AppLayout = styled.div`
  display: flex;

  ${({ phase }) =>
    phase === "running" ? cssForPhase[phase] : cssForPhase["initial"]}
`

const CentralMessageLayout = styled.section`
  display: flex;
  flex-direction: column;
  align-items: center;
  user-select: none;

  font-size: 24px;
  text-align: center;
  color: rgba(0, 0, 0, 0.7);
`

const CentralMessage = ({ children }) => {
  return (
    <CentralMessageLayout>
      {children}
      <LoadingIndicator />
    </CentralMessageLayout>
  )
}

render(<App />, document.getElementById("app"))
